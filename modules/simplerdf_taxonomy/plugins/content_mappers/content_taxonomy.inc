<?php
// $Id$
/**
 * @file
 * Taxonomy field provider.
 */

$plugin = array(
  'title' => t('Content Taxonomy'),
  'field types' => array('content_taxonomy'),
  'map callback' => 'simplerdf_taxonomy_content_mapper_content_taxonomy_map',
  'weight' => -10,
);

/**
 * Map taxonomy ID value to term path.
 */
function simplerdf_taxonomy_content_mapper_content_taxonomy_map($node, $nodeuri, $property, $field, $value) {
  // Map a non-empty value
  if ($value['value']) {
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_get_uri('term', $value['value']));
    return array($s + $p + $o);
  }
}
