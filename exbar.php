<?php
$filepath = isset($argv[1]) ? $argv[1] : __DIR__ . '/samples/sample1.txt';
if (file_exists($filepath) === false || is_readable($filepath) === false) {
    exit("The filepath cannot be opened or read\n");
}

define('COLOR_NONE', 0);
define('COLOR_RED', 1);
define('COLOR_VIOLET', 2);
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
    return array_reduce($apta, function ($carry, $node) use ($label) {
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

function setColor($nodeIdx, $color) {
    global $apta, $changesHistory;
    $changesHistory[] = ['color', $nodeIdx, $apta[$nodeIdx]['color']];
    $apta[$nodeIdx]['color'] = $color;
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
            if ($apta[$i]['children'][$j] === $blueNode) {
                setChild($i, $j, $redNode);
            }
        }
    }
}

function revertChanges($limit = 1) {
    global $apta, $changesHistory;
    while (--$limit >= 0) {
        $change = array_pop($changesHistory);
        if ($change[0] === 'label') {
            $apta[$change[1]]['label'] = $change[2];
        } else if ($change[0] === 'child') {
            $apta[$change[1]]['children'][$change[2]] = $change[3];
        } else if ($change[0] === 'color') {
            $apta[$change[1]]['color'] = $change[2];
        }
    }
}

function indexNodes($nodeIdx, &$index, &$visited, &$map, &$acceptedStates, &$rejectedStates) {
    global $apta, $sofAlphabet;
    $map[$index] = $nodeIdx;
    if ($apta[$nodeIdx]['label'] === LABEL_ACCEPTED) {
        $acceptedStates[] = $index;
    }
    if ($apta[$nodeIdx]['label'] === LABEL_REJECTED) {
        $rejectedStates[] = $index;
    }
    $apta[$nodeIdx]['index'] = $index++;
    $visited[$nodeIdx] = $nodeIdx;
    for ($i = 0; $i < $sofAlphabet; ++$i) {
        if (
            $apta[$nodeIdx]['children'][$i] !== NO_CHILD
            && array_key_exists($apta[$nodeIdx]['children'][$i], $visited) === false
        ) {
            indexNodes($apta[$nodeIdx]['children'][$i], $index, $visited, $map, $acceptedStates, $rejectedStates);
        }
    }
}

function getDfa($result) {
    global $apta, $sofAlphabet;
    $dfa = $result['nofNodes'] . "\n";
    $dfa .= $sofAlphabet . "\n";
    $dfa .= trim(count($result['acceptedStates']) . ' ' . implode(' ', $result['acceptedStates'])) . "\n";
    $dfa .= trim(count($result['rejectedStates']) . ' ' . implode(' ', $result['rejectedStates'])) . "\n";
    foreach ($result['map'] as $index => $nodeIdx) {
        for ($i = 0; $i < $sofAlphabet; ++$i) {
            $dfa .= ($i > 0 ? ' ' : '') . ($apta[$nodeIdx]['children'][$i] === NO_CHILD ? NO_CHILD : $apta[$apta[$nodeIdx]['children'][$i]]['index']);
        }
        $dfa .= "\n";
    }
    return $dfa;
}

/* This part contains the EXBAR algorithm */

$maxNofRedNodes = 1;

function pickBlueNode($cutoff = 0) {
    global $apta, $nofPossibleMerges;
    $blueNodes = array_unique(array_reduce($apta, function ($carry, $node) use ($apta) {
        return array_merge($carry, $node['color'] !== COLOR_RED ? [] : array_filter($node['children'], function ($child) use ($apta) {
            return $child !== NO_CHILD && $apta[$child]['color'] === COLOR_NONE;
        }));
    }, []));
    echo '[' . implode(', ' , $blueNodes) . ']' . PHP_EOL;
    if (count($blueNodes) === 0) {
        return [$cutoff, -1];
    }
    foreach ($blueNodes as $nodeIdx) {
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
    setFatherPoint($blueNode, $redNode);
    try {
        walkit($blueNode, $redNode);
    } catch (Exception $exception) {
        echo '[MERGE FAILED]' . PHP_EOL;
        return false;
    }
    return true;
}

function exbarSearch() {
    global $apta, $maxNofRedNodes, $changesHistory;
    echo '[EXBAR] No. red nodes: ' . getNofRedNodes() . ', max. no. red nodes: ' . $maxNofRedNodes . PHP_EOL;
    if (getNofRedNodes() > $maxNofRedNodes) {
        echo '[LIMIT OF RED NODES EXCEEDED]' . PHP_EOL;
        return;
    }
    updateNofPossibleMerges();
    list($cutoff, $blueNode) = pickBlueNode();
    echo '[BLUE NODE] The picked node: ' . $blueNode . PHP_EOL;
    if ($blueNode === -1) {
        throw new Exception();
    }
    $redNodes = array_filter($apta, function ($node) { return $node['color'] === COLOR_RED; });
    foreach (array_keys($redNodes) as $redNode) {
        echo '[MERGE] The red node: ' . $redNode . PHP_EOL;
        $nofChangesBefore = count($changesHistory);
        if (tryMerge($blueNode, $redNode)) {
            echo '[MERGE COMPLETED]' . PHP_EOL;
            setColor($blueNode, COLOR_VIOLET);
            exbarSearch();
        }
        $nofChangesAfter = count($changesHistory);
        revertChanges($nofChangesAfter - $nofChangesBefore);
    }
    echo '[RECOLOR] The new red node: ' . $blueNode . PHP_EOL;
    setColor($blueNode, COLOR_RED);
    exbarSearch();
    revertChanges(1);
}

function exbarSearch2() {
    global $apta, $maxNofRedNodes, $changesHistory;
    echo '[EXBAR] No. red nodes: ' . getNofRedNodes() . ', max. no. red nodes: ' . $maxNofRedNodes . PHP_EOL;
    if (getNofRedNodes() > $maxNofRedNodes) {
        echo '[LIMIT OF RED NODES EXCEEDED]' . PHP_EOL;
        return;
    }
    $redNodes = [];
    $blueNodes = [];
    foreach ($apta as $nodeIdx => $node) {
        if ($node['color'] === COLOR_RED) {
            $redNodes[$nodeIdx] = $nodeIdx;
            foreach ($node['children'] as $blueNode) {
                if ($blueNode !== NO_CHILD && $apta[$blueNode]['color'] === COLOR_NONE) {
                    $blueNodes[$blueNode] = $blueNode;
                }
            }
        }
    }
    if (count($blueNodes) === 0) {
        throw new Exception();
    }
    // @todo: sorting blue nodes
    foreach ($blueNodes as $blueNode) {
        echo '[BLUE NODE] The picked node: ' . $blueNode . PHP_EOL;
        foreach ($redNodes as $redNode) {
            echo '[MERGE] The red node: ' . $redNode . PHP_EOL;
            $nofChangesBefore = count($changesHistory);
            if (tryMerge($blueNode, $redNode)) {
                echo '[MERGE COMPLETED]' . PHP_EOL;
                setColor($blueNode, COLOR_VIOLET);
                exbarSearch2();
            }
            $nofChangesAfter = count($changesHistory);
            revertChanges($nofChangesAfter - $nofChangesBefore);
        }
        echo '[RECOLOR] The new red node: ' . $blueNode . PHP_EOL;
        setColor($blueNode, COLOR_RED);
        exbarSearch2();
        revertChanges(1);
    }
}

$start = microtime(true);
while (true) {
    try {
        echo '[RESET] Max. of red nodes: ' . $maxNofRedNodes . PHP_EOL;
        exbarSearch();
        $maxNofRedNodes++;
    } catch (Exception $exception) {
        echo '[SOLUTION HAS BEEN FOUND]' . PHP_EOL;
        echo '- no. nodes before: ' . count($apta) . PHP_EOL;
        echo '- no. nodes after: ' . getNofRedNodes() . PHP_EOL;
        $elapsed = microtime(true) - $start;
        echo '- elapsed time: ' . $elapsed . ' sec.' . PHP_EOL;
        $index = 0;
        $visited = [];
        $map = [];
        $acceptedStates = [];
        $rejectedStates = [];
        indexNodes(0, $index, $visited, $map, $acceptedStates, $rejectedStates);
        $result = [
            'nofNodes' => $index,
            'map' => $map,
            'acceptedStates' => $acceptedStates,
            'rejectedStates' => $rejectedStates
        ];
        echo getDfa($result);
        break;
    }
}

echo '[VERIFICATION]' . PHP_EOL;
$hasIncorrect = false;
foreach (array_slice($lines, 1) as $exampleIdx => $example) {
    $current = 0;
    for ($i = 0; $i < $example[1]; ++$i) {
        $current = $apta[$current]['children'][$example[$i + 2]];
    }
    $isCorrect =
        ($example[0] === 1 && $apta[$current]['label'] === LABEL_ACCEPTED)
        || ($example[0] === 0 && $apta[$current]['label'] === LABEL_REJECTED);
    if ($isCorrect === false) {
        echo '[INCORRECT] The example #' .  $exampleIdx . ' is incorrect' . PHP_EOL;
        $hasIncorrect = true;
    }
}
if ($hasIncorrect === false) {
    echo '- ok' . PHP_EOL;
}
