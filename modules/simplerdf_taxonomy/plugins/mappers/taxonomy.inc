<?php
// $Id$
/**
 * @file
 * Core taxonomy mapper.
 */

/**
 * Core taxonomy mapper plugin definition.
 */
$plugin = array(
  'title' => t('Taxonomy'),
  'description' => t('Core taxonomy term mapping.'),
  'callback fields' => 'simplerdf_taxonomy_mapper_fields',
  'callback map' => 'simplerdf_taxonomy_mapper_map',
);

/**
 * Provide core taxonomy term field.
 */
function simplerdf_taxonomy_mapper_fields($content_type) {
  return array(
    'taxonomy_term' => array(
      'title' => t('Taxonomy term'),
    ),
  );
}

/**
 * Mapping callback.
 */
function simplerdf_taxonomy_mapper_map($node, $nodeuri, $property, $field) {
  if (!empty($node->taxonomy)) {
    $tids = array_keys($node->taxonomy);

    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);

    $triples = array();
    foreach ($tids as $tid) {
      $triples[] = $s + $p + simplerdf_make_object('uri', simplerdf_taxonomy_get_termuri($tid));
    }

    return $triples;
  }
}
