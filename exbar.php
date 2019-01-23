<?php
$filepath = __DIR__ . '/samples/sample2.txt';

define('COLOR_NONE', 0);
define('COLOR_RED', 1);
define('LABEL_NONE', 0);
define('LABEL_ACCEPTED', 1);
define('LABEL_REJECTED', -1);
define('NO_CHILD', -1);

/* This part reads and generates the APTA tree from the file. */

$apta = [];
$flags = FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES;
$lines = array_map(function ($line) {
    return array_map(function ($number) { return (int) $number; }, explode(' ', $line));
}, file($filepath, $flags));
$nofExamples = $lines[0][0];
$sofAlphabet = $lines[0][1];
// creates the apta tree root
$apta[0] = [
    'color' => COLOR_RED,
    'label' => LABEL_NONE,
    'children' => array_fill(0, $sofAlphabet, NO_CHILD)
];
$nodeIdx = 0;
foreach (array_slice($lines, 1) as $example) {
    $current = 0;
    for ($i = 0; $i < $example[1]; ++$i) {
        if ($apta[$current]['children'][$example[$i + 2]] === NO_CHILD) {
            $apta[++$nodeIdx] = [
                'color' => COLOR_NONE,
                'label' => LABEL_NONE,
                'children' => array_fill(0, $sofAlphabet, NO_CHILD)
            ];
            $apta[$current]['children'][$example[$i + 2]] = $nodeIdx;
        }
        $current = $apta[$current]['children'][$example[$i + 2]];
    }
    $apta[$current]['label'] = $example[0] ? LABEL_ACCEPTED : LABEL_REJECTED;
}

/* This part contains helper functions */

$nofPossibleMerges = [];
$changesHistory = [];

function getNofRedNodes() {
    global $apta;
    return array_reduce($apta, function ($carry, $node) {
        return $carry + ($node['color'] === COLOR_RED ? 1 : 0);
    }, 0);
}

function getNofPossibleMerges($label) {
    global $apta;
    return array_reduce($apta, function ($carry, $node) {
        return $carry + ($node['color'] === COLOR_RED && ($node['label'] === LABEL_NONE || $node['label'] === $label) ? 1 : 0);
    }, 0);
}

function updateNofPossibleMerges() {
    global $nofPossibleMerges;
    foreach ([LABEL_NONE, LABEL_ACCEPTED, LABEL_REJECTED] as $label) {
        $nofPossibleMerges[$label] = getNofPossibleMerges($label);
    }
}

function setLabel($nodeIdx, $label) {
    global $apta, $changesHistory;
    $changesHistory[] = ['label', $nodeIdx, $apta[$nodeIdx]['label']];
    $apta[$nodeIdx]['label'] = $label;
}

function setChild($nodeIdx, $childIdx, $child) {
    global $apta, $changesHistory;
    $changesHistory[] = ['child', $nodeIdx, $childIdx, $apta[$nodeIdx]['children'][$childIdx]];
    $apta[$nodeIdx]['children'][$childIdx] = $child;
}

function setFatherPoint($blueNode, $redNode) {
    global $apta, $sofAlphabet;
    for ($i = 0; $i < count($apta); ++$i) {
        for ($j = 0; $j < $sofAlphabet; ++$j) {
            if ($apta[$i]['children'] == $blueNode) {
                setChild($i, $j, $redNode);
            }
        }
    }
}

function revertChanges() {
    global $apta, $changesHistory;
    foreach ($changesHistory as $change) {
        if ($change[0] === 'label') {
            $apta[$change[1]]['label'] = $change[2];
        } else if ($change[0] === 'child') {
            $apta[$change[1]]['children'][$change[2]] = $change[3];
        }
    }
}

/* This part contains the EXBAR algorithm */

$maxNofRedNodes = 1;

function pickBlueNode($cutoff = 0) {
    global $apta, $nofPossibleMerges;
    $blueNodes = array_unique(array_reduce($apta, function ($carry, $node) {
        return array_merge($carry, $node['color'] !== COLOR_RED ? [] : array_filter($node['children'], function ($child) { return $child !== NO_CHILD; }));
    }, []));
    if (count($blueNodes) === 0) {
        return [$cutoff, -1];
    }
    foreach ($blueNodes as $nodeIdx) {
        if ($apta[$nodeIdx]['color'] !== COLOR_NONE) {
            continue;
        }
        if ($nofPossibleMerges[$apta[$nodeIdx]['label']] > $cutoff) {
            continue;
        }
        return [$cutoff, $nodeIdx];
    }
    return pickBlueNode($cutoff + 1);
}

function walkit($blueNode, $redNode) {
    global $apta, $sofAlphabet;
    if ($apta[$blueNode]['label'] !== LABEL_NONE) {
        if ($apta[$redNode]['label'] !== LABEL_NONE) {
            if ($apta[$blueNode]['label'] !== $apta[$redNode]['label']) {
                throw new Exception();
            }
        } else {
            setLabel($redNode, $apta[$blueNode]['label']);
        }
    }
    for ($i = 0; $i < $sofAlphabet; ++$i) {
        if ($apta[$blueNode]['children'][$i] !== NO_CHILD) {
            if ($apta[$redNode]['children'][$i] !== NO_CHILD) {
                walkit($apta[$blueNode]['children'][$i], $apta[$redNode]['children'][$i]);
            } else {
                setChild($redNode, $i, $apta[$blueNode]['children'][$i]);
            }
        }
    }
}

function tryMerge($blueNode, $redNode) {
    global $changesHistory;
    $changesHistory = [];
    setFatherPoint($blueNode, $redNode);
    try {
        walkit($blueNode, $redNode);
    } catch (Exception $exception) {
        echo '[MERGE FAILED REVERTED CHANGES]' . PHP_EOL;
        revertChanges();
        return false;
    }
    return true;
}

function exbarSearch() {
    global $apta, $maxNofRedNodes;
    echo '[EXBAR] No. red nodes: ' . getNofRedNodes() . ', max. no. red nodes: ' . $maxNofRedNodes . PHP_EOL;
    if (getNofRedNodes() > $maxNofRedNodes) {
        return;
    }
    list($cutoff, $blueNode) = pickBlueNode();
    echo '[BLUE NODE] The picked node: ' . $blueNode . PHP_EOL;
    if ($blueNode === -1) {
        throw new Exception();
    }
    if ($cutoff === 0) {
        echo '[RECOLOR] The new red node: ' . $blueNode . PHP_EOL;
        $apta[$blueNode]['color'] = COLOR_RED;
        exbarSearch();
        return;
    }
    $redNodes = array_filter($apta, function ($node) { return $node['color'] === COLOR_RED; });
    foreach (array_keys($redNodes) as $redNode) {
        echo '[MERGE] The red node: ' . $redNode . PHP_EOL;
        if (tryMerge($blueNode, $redNode)) {
            echo '[MERGE COMPLETED]' . PHP_EOL;
            exbarSearch();
            return;
        }
    }
    echo '[RECOLOR] The new red node: ' . $blueNode . PHP_EOL;
    $apta[$blueNode]['color'] = COLOR_RED;
    return;
}

$limit = 4;
while (--$limit > 0) {
    try {
        updateNofPossibleMerges();
        exbarSearch();
        $maxNofRedNodes++;
    } catch (Exception $exception) {
        echo '[END]' . PHP_EOL;
        break;
    }
}

