<?php
// $Id$
/**
 * @file
 * Core user mapper.
 */

/**
 * User mapper plugin definition.
 */
$plugin = array(
  'title' => t('Node: core user'),
  'object types' => array('node'),
  'description' => t('Node user mapping.'),
  'fields callback' => 'simplerdf_mapper_node_user_fields',
  'map callback' => 'simplerdf_mapper_node_user_map',
  'weight' => -17,    // Put right below core node mapper
);

/**
 * Provide user fields.
 */
function simplerdf_mapper_node_user_fields($content_type) {
  return array(
    'author' => array(
      'title' => t('Author'),
    ),
  );
}

/**
 * Mapping callback.
 */
function simplerdf_mapper_node_user_map($object_type = 'node', $node, $uri, $property, $field) {
  $triples = array();
  if ($node->uid) {
    // Map non-anonymous author
    $s = simplerdf_make_subject('uri', $uri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_get_uri('user', $node->uid, user_load($node->uid)));
    $triples[] = $s + $p + $o;

    return $triples;
  }
}
