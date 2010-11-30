<?php
// $Id$
/**
 * @file
 * Features integration.
 */

/**
 * Define namespace exportable.
 */
function _simplerdf_namespaces_features_api() {
  $path = drupal_get_path('module', 'simplerdf_namespaces');
  return array(
    'simplerdf_namespaces' => array(
      'name' => t('Simple RDF namespaces'),
      'default_hook' => 'simplerdf_default_namespaces',
      'features_source' => TRUE,
      'duplicates' => FEATURES_DUPLICATES_ALLOWED,
      'file' => $path . '/simplerdf_namespaces.features.inc',
    ),
  );
}

/**
 * Implements hook_features_export() for simplerdf_namespaces.
 */
function simplerdf_namespaces_features_export($data, &$export, $module_name) {
  foreach ($data as $name) {
    $export['features']['simplerdf_namespaces'][$name] = $name;
  }
  $export['dependencies']['simplerdf_namespaces'] = 'simplerdf_namespaces';
  return array();
}

/**
 * Implements hook_features_export_options() for simplerdf_namespaces.
 */
function simplerdf_namespaces_features_export_options() {
  $namespaces = simplerdf_namespaces_load();
  $options = array();
  foreach ($namespaces as $name => $ns) {
    $options[$name] = $name . ' : ' . $ns->uri;
  }
  return $options;
}

/**
 * Implements hook_features_export_render() for simplerdf_namespaces.
 */
function simplerdf_namespaces_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  foreach ($data as $name) {
    $ns = simplerdf_namespaces_load($name);
    if ($ns) {
      // Export prefix and URI by name
      $code[$name] = array(
        'prefix' => $ns->prefix,
        'uri' => $ns->uri,
      );
    }
  }

  $code = '  return ' . features_var_export($code, '  ') . ';';
  return array('simplerdf_default_namespaces' => $code);
}

/**
 * Implements hook_features_revert() for simplerdf_namespaces.
 */
function simplerdf_namespaces_features_revert($module) {
  simplerdf_namespaces_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild() for simplerdf_namespaces.
 */
function simplerdf_namespaces_features_rebuild($module) {
  if ($namespaces = features_get_default('simplerdf_namespaces', $module)) {
    foreach ($namespaces as $name => $ns) {
      $ns = array('name' => $name) + $ns;
      drupal_write_record('simplerdf_namespaces', $ns, 'name');
      if (!db_affected_rows()) {
        drupal_write_record('simplerdf_namespaces', $ns);
      }
    }
    _simplerdf_namespaces_build(NULL, TRUE);
  }
}
