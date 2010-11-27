<?php
// $Id$
/**
 * @file
 * Aliased path URI generator.
 */

/**
 * Node alias generator plugin definition.
 */
$plugin = array(
  'title' => t('Aliased path'),
  'description' => t('Use the absolute URL based on the node\'s alias as the URI.'),
  'callback generate' => '_simplerdf_get_nodeuri',
  'callback generate arguments' => array('alias'),
);
