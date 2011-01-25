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
  'object types' => array('user'),
  'description' => t('Basic properties belonging to a user.'),
  'weight' => -20,
  'fields callback' => 'simplerdf_mapper_user_fields',
  'map callback' => 'simplerdf_mapper_user_map',
);

/**
 * Provide user fields.
 */
function simplerdf_mapper_user_fields($object_type = 'user', $classifier = NULL) {
  $fields['rdftype'] = array(
    'title' => t('RDF class (rdf:type)'),
  );
  $fields['name'] = array(
    'title' => t('User name'),
  );
  $fields['mail'] = array(
    'title' => t('Email address'),
  );

  return $fields;
}

/**
 * Mapping callback.
 */
function simplerdf_mapper_user_map($object_type = 'user', $user, $uri, $property, $field) {
  $field_name = $field['name'];

  $s = simplerdf_make_subject('uri', $uri);
  $p = simplerdf_make_predicate($property);

  $triples = array();
  switch ($field_name) {
    // RDF type
    case 'rdftype':
      $o = simplerdf_make_object('uri', $property);
      $triples[] = $s + simplerdf_make_predicate('rdf:type') + $o;
      break;

    // Single literal values
    case 'name':
      if (!empty($user->$field_name)) {
        $o = simplerdf_make_object('literal', $user->$field_name);
        $triples[] = $s + $p + $o;
      }
      break;

    // Email address
    case 'mail':
      if (!empty($user->$field_name)) {
        $o = simplerdf_make_object('uri', 'mailto:' . $user->$field_name);
        $triples[] = $s + $p + $o;
      }
      break;
  }

  return $triples;
}
