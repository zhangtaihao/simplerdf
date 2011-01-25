<?php
// $Id$
/**
 * @file
 * Nodereference field provider.
 */

$plugin = array(
  'title' => t('Nodereference'),
  'field types' => array('nodereference'),
  'map callback' => 'simplerdf_content_mapper_nodereference_map',
  'weight' => -10,
);

/**
 * Map nodereference nid value.
 */
function simplerdf_content_mapper_nodereference_map($node, $uri, $property, $field, $value) {
  // Map a non-empty value
  if ($value['nid'] && ($refnode = node_load($value['nid'])) && $refnode->status) {
    $s = simplerdf_make_subject('uri', $uri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_get_uri('node', $value['nid'], $refnode));
    return array($s + $p + $o);
  }
}
