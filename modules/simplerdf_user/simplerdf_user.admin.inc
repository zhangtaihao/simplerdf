<?php
// $Id$
/**
 * @file
 * Taxonomy mapping administration UI.
 */

/**
 * User mappings form.
 */
function simplerdf_user_admin_mappings_form($form_state) {
  drupal_set_title(t('Manage user RDF mappings'));

  $form['help'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#value' => t('Enter one or more RDF property mapping (separated by commas) for each user field listed below. Namespace prefixes defined for this site are listed in the table below (open the "RDF namespaces" fieldset to view). You may use these prefixes in the property mappings. Note: specified mappings will not be checked against their vocabularies.'),
    '#suffix' => '</p>',
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('simplerdf_namespaces'),
  );

  // Load mappings
  $rdf_mappings = simplerdf_user_get_mappings();

  // Build mappings
  $form['mappings'] = array(
    '#theme' => 'simplerdf_admin_basic_mappings_table',
  );
  $fields = simplerdf_user_get_fields($vid);
  foreach ($fields as $name => $field) {
    $mapping = array(
      '#field_title' => $field,
      '#field_name' => $name,
      'properties' => array('#tree' => TRUE),
    );
    // Mapping textfield
    $mapping['properties'][$name] = array(
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 100,
      '#default_value' => implode(', ', $rdf_mappings[$name]),
    );

    $form['mappings'][] = $mapping;
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save mappings'),
  );

  return $form;
}

/**
 * Mappings form submit handler.
 */
function simplerdf_user_admin_mappings_form_submit(&$form, &$form_state) {
  $values = $form_state['values'];

  $mappings = array();
  $fields = simplerdf_user_get_fields();
  foreach (array_keys($fields) as $name) {
    if (!empty($values['properties'][$name])) {
      $properties = array_filter(array_map('trim', preg_split('/\s*,\s*/', $values['properties'][$name])));
      if (!empty($properties)) {
        $mappings[$name] = $properties;
      }
    }
  }
  simplerdf_user_save_mappings($mappings);
  simplerdf_clear_cache('user');
}
