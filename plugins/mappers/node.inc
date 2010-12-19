<?php
// $Id$
/**
 * @file
 * Core node mapper.
 */

/**
 * Core node mapper plugin definition.
 */
$plugin = array(
  'title' => t('Node'),
  'object types' => array('node'),
  'description' => t('Basic node properties for the basic description of a node.'),
  'weight' => -20,
  'callback fields' => 'simplerdf_mapper_node_fields',
  'callback map' => 'simplerdf_mapper_node_map',
);

/**
 * Provide core node fields.
 */
function simplerdf_mapper_node_fields($object_type = 'node', $content_type) {
  $fields = array();
  $type = node_get_types('type', $content_type);

  // Node type
  $fields['rdftype'] = array(
    'title' => t('RDF class (rdf:type)'),
  );

  // Node title
  if ($type->has_title) {
    $fields['title'] = array(
      'title' => $type->title_label,
    );
  }

  // Node body
  if ($type->has_body) {
    $fields['body'] = array(
      'title' => $type->body_label,
    );
  }

  // Node created date
  $fields['created'] = array(
    'title' => t('Creation date'),
  );

  // Node changed date
  $fields['changed'] = array(
    'title' => t('Modification date'),
  );

  return $fields;
}

/**
 * Mapping callback.
 */
function simplerdf_mapper_node_map($object_type = 'node', $node, $uri, $property, $field) {
  $field_name = $field['name'];

  $s = simplerdf_make_subject('uri', $uri);
  $p = simplerdf_make_predicate($property);

  switch ($field_name) {
    // Type
    case 'rdftype':
      $p = simplerdf_make_predicate('rdf:type');
      $o = simplerdf_make_object('uri', $property);
      break;

    // Text fields
    case 'title':
    case 'body':
      $o = simplerdf_make_object('literal', $node->$field_name);
      break;

    // Timestamps
    case 'created':
    case 'changed':
      $time = date('c', $node->$field_name);
      $o = simplerdf_make_object('literal', $time, 'xsd:dateTime');
      break;
  }

  if ($o) {
    return array($s + $p + $o);
  }
}
