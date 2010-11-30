<?php
// $Id$
/**
 * @file
 * Features integration.
 */

/**
 * Define custom field exportable.
 */
function _simplerdf_custom_fields_features_api() {
  $path = drupal_get_path('module', 'simplerdf_custom_fields');
  return array(
    'simplerdf_custom_fields' => array(
      'name' => t('Simple RDF custom fields'),
      'default_hook' => 'simplerdf_default_custom_fields',
      'features_source' => TRUE,
      'file' => $path . '/simplerdf_custom_fields.features.inc',
    ),
  );
}

/**
 * Implements hook_features_export() for simplerdf_custom_fields.
 */
function simplerdf_custom_fields_features_export($data, &$export, $module_name) {
  foreach ($data as $name) {
    $export['features']['simplerdf_custom_fields'][$name] = $name;
  }
  $export['dependencies']['simplerdf_custom_fields'] = 'simplerdf_custom_fields';
  return array();
}

/**
 * Implements hook_features_export_options() for simplerdf_custom_fields.
 */
function simplerdf_custom_fields_features_export_options() {
  $fields = simplerdf_custom_fields_load();
  $options = array();
  foreach ($fields as $name => $field) {
    $options[$name] = $field->title . ' (' . $name . ')';
  }
  return $options;
}

/**
 * Implements hook_features_export_render() for simplerdf_custom_fields.
 */
function simplerdf_custom_fields_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  foreach ($data as $name) {
    $field = simplerdf_custom_fields_load($name);
    if ($field) {
      // Export prefix and URI by name
      $code[$name] = array(
        'title' => $field->title,
        'type' => $field->type,
        'settings' => $field->settings,
      );
    }
  }

  $code = '  return ' . features_var_export($code, '  ') . ';';
  return array('simplerdf_default_custom_fields' => $code);
}

/**
 * Implements hook_features_revert() for simplerdf_custom_fields.
 */
function simplerdf_custom_fields_features_revert($module) {
  simplerdf_custom_fields_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild() for simplerdf_custom_fields.
 */
function simplerdf_custom_fields_features_rebuild($module) {
  if ($fields = features_get_default('simplerdf_custom_fields', $module)) {
    foreach ($fields as $name => $field) {
      $field = array('name' => $name) + $field;
      drupal_write_record('simplerdf_custom_fields', $field, 'name');
      if (!db_affected_rows()) {
        drupal_write_record('simplerdf_custom_fields', $field);
      }
    }
    _simplerdf_custom_fields_build(NULL, TRUE);
  }
}
