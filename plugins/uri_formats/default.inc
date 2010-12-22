<?php
// $Id$
/**
 * @file
 * Default path URI for node and user.
 */

/**
 * Default node/user URI format plugin definition.
 */
$plugin = array(
  'title' => t('Default'),
  'description' => t('Use the object\'s path or its alias as the URI, e.g. "http://www.example.com/node/123".'),
  'object types' => array('node', 'user'),
  'aliasable' => TRUE,
  'internal' => TRUE,
  'callback format' => 'simplerdf_get_default_uri',
  'weight' => -20,
);
