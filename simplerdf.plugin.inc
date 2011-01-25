<?php
/**
 * @file
 * Simple RDF plugin architecture. This file is loaded with the main module file.
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function simplerdf_ctools_plugin_api($owner, $api) {
  // Default plugins
  if ($owner == 'simplerdf') {
    switch ($api) {
      case 'mappers':
        return array('version' => 1);

      case 'content_mappers':
        return array('version' => 1);

      case 'uri_formats':
        return array('version' => 1);
    }
  }
}

/**
 * Implements hook_ctools_plugin_directory().
 */
function simplerdf_ctools_plugin_directory($owner, $type) {
  // Default plugins
  if ($owner == 'simplerdf') {
    return 'plugins/' . $type;
  }
}

/**
 * Implements hook_ctools_plugin_TYPE() for mappers.
 */
function simplerdf_ctools_plugin_mappers() {
  return array(
    'cache' => TRUE,
    'cache table' => 'cache_simplerdf',
    'use hooks' => TRUE,
    'process' => 'simplerdf_mappers_process',
  );
}

/**
 * Mapper plugin processor.
 */
function simplerdf_mappers_process(&$plugin, $info) {
  // Add default values
  $plugin += array(
    'title' => '',
    'object types' => array(),
    'description' => '',
    'weight' => 0,
    'fields callback' => NULL,
    'map callback' => NULL,
  );
}

/**
 * Implements hook_ctools_plugin_TYPE() for content_mappers.
 */
function simplerdf_ctools_plugin_content_mappers() {
  return array(
    'cache' => TRUE,
    'cache table' => 'cache_simplerdf',
    'use hooks' => TRUE,
    'process' => 'simplerdf_content_mappers_process',
  );
}

/**
 * Content mapper plugin processor.
 */
function simplerdf_content_mappers_process(&$plugin, $info) {
  // Add default values
  $plugin += array(
    'weight' => 0,
    'field modules' => array(),
    'field types' => array(),
    'fields callback' => '_simplerdf_content_mappers_default_fields',
    'map callback' => NULL,
  );
}

/**
 * Default field callback.
 * This returns the basic content field with its name and label title.
 */
function _simplerdf_content_mappers_default_fields($content_field) {
  $field = array(
    'title' => $content_field['widget']['label'],
  );
  return array($content_field['field_name'] => $field);
}

/**
 * Implements hook_ctools_plugin_TYPE() for uri_formats.
 */
function simplerdf_ctools_plugin_uri_formats() {
  return array(
    'cache' => TRUE,
    'cache table' => 'cache_simplerdf',
    'use hooks' => TRUE,
    'process' => 'simplerdf_uri_formats_process',
  );
}

/**
 * Node URI generator plugin processor.
 */
function simplerdf_uri_formats_process(&$plugin, $info) {
  $path = drupal_get_path('module', $plugin['module']);

  // Add default values
  $plugin += array(
    'title' => '',
    'description' => '',
    'object types' => array(),
    'aliasable' => FALSE,
    'internal' => FALSE,
    'weight' => 0,
    'format callback' => NULL,
    'settings defaults' => array(),
    'settings form' => NULL,
    'settings form validate' => NULL,
  );
}

/**
 * Invoke a plugin callback.
 */
function simplerdf_plugin_invoke($plugin, $op) {
  // Preprocess function arguments
  $args = func_get_args();
  $plugin = array_shift($args);
  $op = array_shift($args);

  // Get callback CTools-style
  ctools_include('plugins');
  $callback = ctools_plugin_get_function($plugin, $op);
  if (!$callback) {
    // Invalid callback
    return;
  }

  // Prepare callback arguments
  if (is_array($plugin[$op]) && is_array($plugin[$op]['arguments'])) {
    $args = array_merge($args, $plugin[$op]['arguments']);
  }

  // Invoke callback
  return call_user_func_array($callback, $args);
}

/**
 * Get plugins by type and ID. This function wraps around ctools_get_plugins().
 *
 * @param $type
 *   The type identifier of the plugin.
 * @param $id
 *   If specified, return only information about plugin with this identifier.
 *
 * @return
 *   Specified plugin if $id is given; otherwise an array of plugins.
 */
function simplerdf_get_plugins($type, $id = NULL) {
  ctools_include('plugins');
  $plugins = ctools_get_plugins('simplerdf', $type, $id);
  if (!isset($id) && is_array($plugins) && function_exists('ctools_plugin_sort')) {
    uasort($plugins, 'ctools_plugin_sort');
  }
  return $plugins;
}

/**
 * Get a URI plugin.
 *
 * @param string $type
 *   Object type, e.g. node.
 *
 * @return
 *   Active URI plugin for the type, or the only plugin for that type if no
 *   active format is specified.
 */
function simplerdf_get_uri_format_plugin($type) {
  static $type_plugins;

  // Get active format
  $formats = (array) variable_get('simplerdf_uri_formats', array());
  $format = $formats[$type];
  if ($format) {
    return simplerdf_get_plugins('uri_formats', $format);
  }
  else {
    // Walk through all plugins and group by object type
    if (!isset($type_plugins)) {
      $type_plugins = array();
      $plugins = simplerdf_get_plugins('uri_formats');
      foreach ($plugins as $plugin_name => $plugin) {
        if ($plugin['object types']) {
          // Walk through only formats for specific types
          foreach ($plugin['object types'] as $object_type) {
            $type_plugins[$object_type][$plugin_name] = $plugin;
          }
        }
      }
    }
    // Find the only plugin for that type
    if (is_array($type_plugins[$type]) && count($type_plugins[$type]) == 1) {
      $plugins = array_values($type_plugins[$type]);
      // Set plugin as default and return
      $formats[$type] = $plugins[0]['name'];
      variable_set('simplerdf_uri_formats', $formats);
      return $plugins[0];
    }
  }
}
