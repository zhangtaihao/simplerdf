<?php
// $Id$
/**
 * @file
 * ARC2 integration.
 */

/** ARC2 library path. If 'libraries' has not been loaded, assume libraries live under 'sites/all/libraries'. */
define('SIMPLERDF_ARC_PATH', function_exists('libraries_get_path') ? libraries_get_path('arc') : 'sites/all/libraries/arc');

/**
 * Include the ARC2 class. To
 */
function simplerdf_arc_include() {
  $arc2 = SIMPLERDF_ARC_PATH . '/ARC2.php';

  // Check that ARC2 exists
  if (file_exists($arc2)) {
    require_once $arc2;
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Get a list of ARC2 formats.
 *
 * @return
 *   An array of format names keyed by names, or NULL if ARC2 is not initialized.
 */
function simplerdf_arc_get_formats() {
  if (class_exists('ARC2')) {
    static $formats;
    if (!isset($formats)) {
      $formats = array();
      $methods = get_class_methods('ARC2');
      // Scan class for serializer factory methods
      foreach ($methods as $method) {
        if (preg_match('/^get(.+)Serializer$/', $method, $match)) {
          $format = $match[1];
          $formats[$format] = $format;
        }
      }
      ksort($formats);
    }
    return $formats;
  }
}

/**
 * Get ARC2 serializer for format.
 *
 * @param string $format
 *   Name of format as given in simplerdf_arc_get_formats(). If not specified,
 *   the active format from settings will be used.
 * @param $conf
 *   Configuration parameters for ARC2 classes. For example, $conf can be an
 *   array with key 'ns' containing an array of namespaces keyed by prefixes.
 *
 * @return ARC2_Class
 *   Serializer object, or NULL if specified format is invalid.
 */
function simplerdf_arc_get_serializer($format = NULL, $conf = '') {
  if (!isset($format)) {
    $format = variable_get('simplerdf_format', SIMPLERDF_DEFAULT_FORMAT);
  }
  $factory_method = 'get' . $format . 'Serializer';
  if (method_exists('ARC2', $factory_method)) {
    return ARC2::$factory_method($conf);
  }
}
