<?php
// $Id$
/**
 * @file
 * Default field provider. Maps the 'value' key in content field values.
 */

$plugin = array(
  'title' => t('Default'),
  'callback map' => 'simplerdf_content_mapper_default_map',
  'weight' => -10,
);

/**
 * Map basic field value.
 */
function simplerdf_content_mapper_default_map($node, $nodeuri, $property, $field, $value) {
  // Map a non-null value
  if (isset($value['value'])) {
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);
    $o = simplerdf_make_object('literal', $value['value']);
    return array($s + $p + $o);
  }
}
