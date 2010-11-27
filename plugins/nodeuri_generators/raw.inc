<?php
// $Id$
/**
 * @file
 * Raw path URI generator.
 */

/**
 * Raw node URI generator plugin definition.
 */
$plugin = array(
  'title' => t('Raw path'),
  'description' => t('Use the node\'s raw path as the URI, e.g. "http://www.example.com/node/123".'),
  'callback generate' => '_simplerdf_get_nodeuri',
  'callback generate arguments' => array('raw'),
);
