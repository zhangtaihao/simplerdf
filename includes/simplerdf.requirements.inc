<?php
// $Id$
/**
 * @file
 * Module requirements implementation.
 */

/**
 * Implements hook_requirements().
 */
function simplerdf_requirements($phase) {
  $includes = drupal_get_path('module', 'simplerdf') . '/includes';
  require_once $includes . '/arc.inc';
  simplerdf_arc_include();

  $t = get_t();

  $requirements = array();
  switch ($phase) {
    case 'install':
      if (!class_exists('ARC2')) {
        // Missing ARC2
        $requirements['simplerdf_arc2'] = array(
          'title' => $t('Simple RDF: ARC2'),
          'description' => $t('The ARC2 library is missing. Please install it under the sites/all/libraries folder.'),
          'severity' => REQUIREMENT_ERROR,
        );
      }
      break;
    case 'runtime':
      if (class_exists('ARC2')) {
        // Show ARC2 version
        $requirements['arc2'] = array(
          'title' => $t('Simple RDF: ARC2'),
          'value' => ARC2::getVersion(),
          'description' => $t('The ARC2 library is missing. Please install it under the sites/all/libraries folder.'),
          'severity' => REQUIREMENT_OK,
        );
      }
      else {
        // Missing ARC2
        $requirements['arc2'] = array(
          'title' => $t('Simple RDF: ARC2'),
          'value' => $t('Not found'),
          'description' => $t('The ARC2 library is missing. Please install it under the sites/all/libraries folder.'),
          'severity' => REQUIREMENT_ERROR,
        );
      }
      break;
  }
  return $requirements;
}
