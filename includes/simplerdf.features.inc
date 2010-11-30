<?php
// $Id$
/**
 * @file
 * Features integration.
 */

/**
 * Provide Features exportables.
 */
function _simplerdf_features_api() {
  $inc_path = drupal_get_path('module', 'simplerdf') . '/includes';

  $components['simplerdf_mappings'] = array(
    'name' => t('Simple RDF mappings'),
    'default_hook' => 'simplerdf_default_mappings',
    'default_file' => FEATURES_DEFAULTS_INCLUDED,
    'features_source' => TRUE,
    'file' => $inc_path . '/simplerdf.features.inc',
  );

  return $components;
}

/**
 * Implements hook_features_export() for simplerdf_mappings.
 */
function simplerdf_mappings_features_export($data, &$export, $module_name) {
  // Add component
  foreach ($data as $type) {
    $export['features']['simplerdf_mappings'][$type] = $type;
  }
  // Trace field provider dependencies
  $export['dependencies']['simplerdf'] = 'simplerdf';
  $fields = simplerdf_get_fields($type);
  $plugins = ctools_get_plugins('simplerdf', 'mappers');
  foreach ($fields as $field) {
    $module = $plugins[$field['mapper']]['module'];
    $export['dependencies'][$module] = $module;
  }
  return array();
}

/**
 * Implements hook_features_export_options() for simplerdf_mappings.
 */
function simplerdf_mappings_features_export_options() {
  return node_get_types('names');
}

/**
 * Implements hook_features_export_render() for simplerdf_mappings.
 */
function simplerdf_mappings_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  foreach ($data as $type) {
    // Fetch fields in order
    $fields = array_keys(simplerdf_get_fields($type));
    sort($fields);
    $mappings = simplerdf_get_mappings($type);
    foreach ($fields as $field) {
      // Export mappings
      if (isset($mappings[$field])) {
        $code[$type][$field] = $mappings[$field];
      }
    }
  }

  $code = '  return '. features_var_export($code, '  ') . ';';
  return array('simplerdf_default_mappings' => $code);
}

/**
 * Implements hook_features_revert() for simplerdf_mappings.
 */
function simplerdf_mappings_features_revert($module) {
  simplerdf_mappings_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild() for simplerdf_mappings.
 */
function simplerdf_mappings_features_rebuild($module) {
  if ($mappings = features_get_default('simplerdf_mappings', $module)) {
    // Remove original mappings
    $types = array_keys($mappings);
    db_query('DELETE FROM {simplerdf_mappings} WHERE content_type IN (' . db_placeholders($types, 'varchar') . ')', $types);

    // Store default mappings
    foreach ($mappings as $type => $type_mappings) {
      foreach ($type_mappings as $field => $properties) {
        $mapping = array(
          'content_type' => $type,
          'field' => $field,
          'properties' => $properties,
        );
        drupal_write_record('simplerdf_mappings', $mapping);
      }
      simplerdf_clear_cache('mappings', $type);
    }

    simplerdf_clear_cache('node');
  }
}
