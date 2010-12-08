<?php
// $Id$
/**
 * @file
 * User mapper.
 */

/**
 * User mapper plugin definition.
 */
$plugin = array(
  'title' => t('User'),
  'description' => t('Node user mapping.'),
  'callback fields' => 'simplerdf_user_mapper_fields',
  'callback map' => 'simplerdf_user_mapper_map',
  'weight' => -17,    // Put right below core node mapper
);

/**
 * Provide user fields.
 */
function simplerdf_user_mapper_fields($content_type) {
  return array(
    'author' => array(
      'title' => t('Author'),
    ),
  );
}

/**
 * Mapping callback.
 */
function simplerdf_user_mapper_map($node, $nodeuri, $property, $field) {
  $triples = array();
  if ($node->uid) {
    // Map non-anonymous author
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('uri', simplerdf_user_get_useruri($node->uid));
    $triples[] = $s + $p + $o;

    return $triples;
  }
}
