<?php
// $Id$
/**
 * @file
 * Default field provider. Maps the 'value' key in content field values.
 */

$plugin = array(
  'title' => t('Default'),
  'callback map' => 'simplerdf_content_mapper_default_map',
);

/**
 * Map basic field value.
 */
function simplerdf_content_mapper_default_map($node, $nodeuri, $property, $field, $values) {
  $field_name = $field['name'];
  if (is_array($values) && isset($values[0])) {
    $s = simplerdf_make_subject($nodeuri);
    $p = simplerdf_make_predicate($property);
    $triples = array();

    // Map each non-null value
    foreach ($values as $entry) {
      if (isset($entry['value'])) {
        $triples[] = $s + $p + simplerdf_make_object('literal', $entry['value']);
      }
    }

    return $triples;
  }
}
