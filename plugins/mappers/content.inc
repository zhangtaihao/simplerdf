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
  'description' => t('Basic node properties for the basic description of a field.'),
  'weight' => -10,
  'callback fields' => 'simplerdf_mapper_content_fields',
  'callback map' => 'simplerdf_mapper_content_map',
);

/**
 * Provide CCK fields.
 */
function simplerdf_mapper_content_fields($content_type) {;
  return simplerdf_get_content_fields($content_type);
}

/**
 * Map CCK fields.
 */
function simplerdf_mapper_content_map($node, $nodeuri, $property, $field) {
  static $plugins;
  // Initialize plugins
  if (!$plugins) {
    ctools_include('plugins');
    $plugins = ctools_get_plugins('simplerdf', 'content_fields');
  }

  $mapper = $plugins[$field['content mapper']];
  if (isset($mapper)) {
    // Delegate to the content mapper
    return _simplerdf_invoke('callback map', $mapper, $node, $nodeuri, $property, $field);
  }
}
