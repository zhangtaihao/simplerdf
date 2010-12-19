<?php
// $Id$
/**
 * @file
 * CCK mapper.
 */

/**
 * CCK field mapper plugin.
 */
$plugin = array(
  'title' => t('CCK'),
  'object types' => array('node'),
  'description' => t('CCK content fields.'),
  'weight' => -10,
  'callback fields' => 'simplerdf_mapper_content_fields',
  'callback map' => 'simplerdf_mapper_content_map',
);

/**
 * Provide CCK fields.
 */
function simplerdf_mapper_content_fields($object_type = 'node', $content_type) {
  return simplerdf_get_content_fields($content_type);
}

/**
 * Map CCK fields.
 */
function simplerdf_mapper_content_map($object_type = 'node', $node, $uri, $property, $field) {
  static $plugins;
  // Initialize plugins
  if (!$plugins) {
    ctools_include('plugins');
    $plugins = ctools_get_plugins('simplerdf', 'content_mappers');
  }

  $mapper = $plugins[$field['content mapper']];
  if (isset($mapper)) {
    // Delegate to the content mapper
    $field_name = $field['content field'];
    if (property_exists($node, $field_name)) {
      $values = $node->$field_name;
      // Ignore invalid values
      if (!is_array($values) || count($values) == 1 && !isset($values[0])) {
        $values = array();
      }

      // Map each non-null value
      $triples = array();
      foreach ($values as $delta => $value) {
        if (isset($value)) {
          $result = _simplerdf_invoke($mapper, 'callback map', $node, $uri, $property, $field, $value, $delta);
          if (is_array($result)) {
            $triples = array_merge($triples, $result);
          }
        }
      }
      return $triples;
    }
  }
}
