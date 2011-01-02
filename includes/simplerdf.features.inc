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
  $plugins = simplerdf_get_plugins('mappers');
  $content_plugins = simplerdf_get_plugins('content_mappers');
  $info = _simplerdf_type_info();
  $type_fields = array();
  foreach ($data as $item) {
    // Add component
    $export['features']['simplerdf_mappings'][$item] = $item;

    // Trace field provider dependencies by selected types
    $limit = strcspn($item, '-');
    $type = substr($item, 0, $limit);
    $classifier = (string) substr($item, $limit + 1);

    $fields = simplerdf_get_fields($type, $classifier == '' ? NULL : $classifier);
    foreach ($fields as $field) {
      $mapper = $field['mapper'];
      if ($mapper != 'content') {
        $module = $plugins[$mapper]['module'];
        $export['dependencies'][$module] = $module;
      }
      else {
        // Trace content mapper dependency
        $content_mapper = $field['content mapper'];
        $module = $content_plugins[$content_mapper];
        $export['dependencies'][$module] = $module;
      }
    }
  }
  $export['dependencies']['simplerdf'] = 'simplerdf';
  return array();
}

/**
 * Implements hook_features_export_options() for simplerdf_mappings.
 */
function simplerdf_mappings_features_export_options() {
  $result = db_query("SELECT DISTINCT type, classifier FROM {simplerdf_mappings} ORDER BY type, classifier");
  $types = simplerdf_get_types();
  $options = array();
  while ($row = db_fetch_array($result)) {
    if ($types[$row['type']]) {
      $type = $row['type'];
      $classifier = (string) $row['classifier'];
      if ($classifier == '') {
        $options[$type] = $types[$type];
      }
      else {
        $id = $type . '-' . $row['classifier'];
        $options[$id] = $types[$type] . ': ' . $row['classifier'];
      }
    }
  }
  return $options;
}

/**
 * Implements hook_features_export_render() for simplerdf_mappings.
 */
function simplerdf_mappings_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  foreach ($data as $item) {
    $limit = strcspn($item, '-');
    $type = substr($item, 0, $limit);
    $classifier = (string) substr($item, $limit + 1);
    $mappings = simplerdf_get_mappings($type, $classifier == '' ? NULL : $classifier);
    // Fetch fields in order
    $fields = array_keys(simplerdf_get_fields($type, $classifier == '' ? NULL : $classifier));
    sort($fields);
    foreach ($fields as $field) {
      // Export mappings
      if (isset($mappings[$field])) {
        $code[$item][$field] = $mappings[$field];
      }
    }
  }

  $output = "  \$mappings = array();\n";
  foreach ($code as $item => $mappings) {
    $output .= "  \$mappings['" . addslashes($item) . "'] = " . features_var_export($mappings, '  ') . ";\n";
  }
  $output .= '  return $mappings;';
  return array('simplerdf_default_mappings' => $output);
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
  if ($all_mappings = features_get_default('simplerdf_mappings', $module)) {
    // Index default mappings and remove original mappings
    $mappings = array();
    foreach ($all_mappings as $item => $class_mappings) {
      $limit = strcspn($item, '-');
      $type = substr($item, 0, $limit);
      $classifier = (string) substr($item, $limit + 1);
      $mappings[$type][$classifier] = $class_mappings;
      db_query("DELETE FROM {simplerdf_mappings} WHERE type = '%s' AND classifier = '%s'", $type, $classifier);
    }

    // Store default mappings
    foreach ($mappings as $type => $type_mappings) {
      foreach ($type_mappings as $classifier => $field_mappings) {
        foreach ($field_mappings as $field => $properties) {
          $mapping = array(
            'type' => $type,
            'classifier' => $classifier,
            'field' => $field,
            'properties' => $properties,
          );
          drupal_write_record('simplerdf_mappings', $mapping);
        }
        simplerdf_clear_cache('mappings', $type, $classifier);
      }
      simplerdf_clear_cache($type);
    }
  }
}
