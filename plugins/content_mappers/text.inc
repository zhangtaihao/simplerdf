<?php
// $Id$
/**
 * @file
 * Text field provider.
 */

$plugin = array(
  'field types' => array('textfield'),
  'callback fields' => 'simplerdf_mapper_text_fields',
  'callback map' => 'simplerdf_mapper_text_map',
);

/**
 * Provide text field.
 */
function simplerdf_mapper_text_fields($content_field) {
  $field_name = $content_field['field_name'];
  $field = array(
    'title' => $content_field['widget']['label'],
  );
  return array($field_name => $fields);
}

/**
 * Map text field.
 */
function simplerdf_mapper_text_map($node, $nodeuri, $property, $field) {
  $s = simplerdf_make_subject($nodeuri);
  $p = simplerdf_make_predicate($property);
  $triples = array();

  $name = $field['name'];
  if (is_array($node->$name)) {
    $values = $node->$name;
    // Map each non-null value
    foreach ($values as $entry) {
      if (isset($entry['value'])) {
        $triples[] = $s + $p + simplerdf_make_object('literal', $entry['value']);
      }
    }
  }

  return $triples;
}
