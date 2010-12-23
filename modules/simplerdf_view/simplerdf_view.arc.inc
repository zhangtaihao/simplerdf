<?php
// $Id$
/**
 * @file
 * Integration with ARC2
 */

/** ARC2 library path. If 'libraries' has not been loaded, assume libraries live under 'sites/all/libraries'. */
define('SIMPLERDF_VIEW_ARC_PATH', function_exists('libraries_get_path') ? libraries_get_path('arc') : 'sites/all/libraries/arc');

/**
 * Include the ARC2 class. To
 */
function simplerdf_view_arc_include() {
  static $init;
  if (!isset($init)) {
    $arc2 = SIMPLERDF_VIEW_ARC_PATH . '/ARC2.php';
    // Check that ARC2 exists
    if (file_exists($arc2)) {
      require_once $arc2;
      $init = TRUE;
    }
    else {
      $init = FALSE;
    }
  }
  return $init;
}

/**
 * Get a list of ARC2 formats.
 *
 * @return
 *   An array of format names keyed by names, or NULL if ARC2 is not initialized.
 */
function simplerdf_view_arc_get_formats() {
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
 *   Name of format as given in simplerdf_view_arc_get_formats().
 * @param $conf
 *   Configuration parameters for ARC2 classes. For example, $conf can be an
 *   array with key 'ns' containing an array of namespaces keyed by prefixes.
 *
 * @return ARC2_Class
 *   Serializer object, or NULL if specified format is invalid.
 */
function simplerdf_view_arc_get_serializer($format, $conf = '') {
  $factory_method = 'get' . $format . 'Serializer';
  if (method_exists('ARC2', $factory_method)) {
    return ARC2::$factory_method($conf);
  }
}

/**
 * Get the MIME type of an ARC2 serializer.
 *
 * @param string $format
 *   Name of format as given in simplerdf_view_arc_get_formats().
 * @param ARC2_Class $serializer
 *   Optional. Fully initialized ARC2 serializer object.
 *
 * @return string
 *   MIME type name string, or NULL if given format is invalid.
 */
function simplerdf_view_arc_get_mime_type($format, ARC2_Class $serializer = NULL) {
  static $types = array();

  // Lazy detect type
  if (!isset($types[$format])) {
    if ($cache = cache_get('arc2_mime_types:' . $format, 'cache_simplerdf')) {
      $types[$format] = $cache->data;
    }
    // Detect type from serializer object
    elseif (isset($serializer) || simplerdf_view_arc_include() && $serializer = simplerdf_view_arc_get_serializer($format)) {
      $type = !empty($serializer->content_header) ? $serializer->content_header : 'text/plain';
      $types[$format] = $type;
      cache_set('arc2_mime_types:' . $format, $type, 'cache_simplerdf');
    }
  }

  return $types[$format] ? $types[$format] : NULL;
}
