<?php
// $Id$
/**
 * @file
 * Number field provider.
 */

$plugin = array(
  'title' => t('Number'),
  'field modules' => array('number'),
  'callback map' => 'simplerdf_content_mapper_number_map',
);

/**
 * Map numeric field value.
 */
function simplerdf_content_mapper_number_map($node, $nodeuri, $property, $field, $value) {
  if (isset($value['value'])) {
    $s = simplerdf_make_subject('uri', $nodeuri);
    $p = simplerdf_make_predicate($property);

    // Provide specific data types
    switch ($field['field type']) {
      case 'number_integer':
        $range = 'xsd:integer';
        break;

      case 'number_decimal':
        $range = 'xsd:decimal';
        break;

      case 'number_float':
        $range = 'xsd:float';
        break;
    }

    $o = simplerdf_make_object('literal', $value['value'], $range);
    return array($s + $p + $o);
  }
}
