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

  // Check that ARC2 exists, or throw an error message if not.
  if (file_exists($arc2)) {
    require_once $arc2;
    return TRUE;
  }
  else {
    drupal_set_message(t('The ARC2 library is missing. Please see status report on how to install it.'), 'error');
    return FALSE;
  }
}

/**
 * Get a list of ARC2 formats.
 */
function simplerdf_arc_get_formats() {
  $path = ARC2::getIncPath('serializer');
  $formats = array();
  foreach (scandir($path) as $dir) {
    if (preg_match('/^ARC2_(.+)Serializer\.php$/', $dir, $matches)) {
      $format = $matches[1];

      // Ignore the default RDF serializer
      if ($format == 'RDF') {
        continue;
      }

      $formats[$format] = $format;
    }
  }
  return $formats;
}
