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

/* This part contains the EXBAR algorithm */

$maxNofRedNodes = 1;

function pickBlueNode($cutoff = 0) {
    global $apta, $nofPossibleMerges;
    $blueNodes = array_unique(array_reduce($pata, function ($carry, $node) {
        return array_merge($carry, $node['color'] !== COLOR_RED ? [] : array_filter($node['children'], function ($child) { return $child !== NO_CHILD; }));
    }, []));
    if (count(blueNodes) === 0) {
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

function exbarSearch() {
    global $apta, $maxNofRedNodes;
    if (getNofRedNodes() <= $maxNofRedNodes) {
        list($cutoff, $blueNode) = pickBlueNode();
        if ($blueNode === -1) {
            throw new Exception();
        }
        if ($cutoff === 0) {
            $apta[$blueNode]['color'] = COLOR_RED;
            exbarSearch();
            return;
        }
        
    }
    return;
}
