<?php
// $Id$
/**
 * @file
 * Features integration.
 */

/**
 * Define RDF term exportable.
 */
function _simplerdf_user_features_api() {
  $path = drupal_get_path('module', 'simplerdf_user');

  // Pseudo-component for exporting mappings
  if (module_exists('strongarm')) {
    $components['simplerdf_user_mappings'] = array(
      'name' => t('Simple RDF user mappings'),
      'feature_source' => TRUE,
    );
  }

  return $components;
}

/**
 * Implements hook_features_export() for simplerdf_user_mappings.
 */
function simplerdf_user_mappings_features_export($data, &$export, $module_name) {
  $pipe = array();
  if (module_exists('strongarm')) {
    if (in_array('simplerdf_user', $data)) {
      // Export user mappings
      $var_name = 'simplerdf_user_mappings';
      if (!is_null(variable_get($var_name, NULL))) {
        $pipe['variable'][$var_name] = $var_name;
      }
    }
    $export['dependencies']['simplerdf_user'] = 'simplerdf_user';
  }
  return $pipe;
}

/**
 * Implements hook_features_export_options() for simplerdf_user_mappings.
 */
function simplerdf_user_mappings_features_export_options() {
  $options = array();

  if (module_exists('strongarm')) {
    // Check mappings exist
    if (simplerdf_user_get_mappings()) {
      $options['simplerdf_user'] = t('User mappings');
    }
  }

  return $options;
}
