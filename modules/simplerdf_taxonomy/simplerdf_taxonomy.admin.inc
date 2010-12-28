<?php
// $Id$
/**
 * @file
 * Taxonomy mapping administration UI.
 */

/**
 * Taxonomy mappings overview page.
 */
function simplerdf_taxonomy_admin_mappings_overview() {
  drupal_add_css(drupal_get_path('module', 'simplerdf') . '/simplerdf.admin.css');

  $output = '<p>' . t('An overview of vocabularies and their RDF type mappings are displayed below. Note that only vocabularies with machine names can be mapped.') . '</p>';

  // Insert namespace legend
  $output .= theme('simplerdf_namespaces');

  $header = array(
    t('Vocabulary'),
    t('Machine name'),
    t('RDF types'),
    t('Operations'),
  );

  $rows = array();
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $vocabulary) {
    $row = array();
    $row[] = check_plain($vocabulary->name);
    $machine_name = _simplerdf_taxonomy_get_vocabulary_name($vocabulary);
    // No machine name
    if (empty($machine_name)) {
      $row[] = array();
      $row[] = array(
        'colspan' => 2,
        'data' => '<em>' . t('Please define a machine name to enable mapping.') . '</em>',
      );
    }
    // Machine name exists
    else {
      $row[] = check_plain($machine_name);

      $mappings = simplerdf_taxonomy_get_mappings($vid);
      $row[] = is_array($mappings['rdftype']) ? implode(', ', $mappings['rdftype']) : '';

      $row[] = l(t('manage mappings'), 'admin/content/taxonomy/' . $vocabulary->vid . '/simplerdf');
    }

    $rows[] = $row;
  }

  $output .= theme('table', $header, $rows, array('attributes' => array('id' => 'simplerdf-taxonomy-mappings-overview', 'class' => 'simplerdf-admin-listing-table')));

  return $output;
}

/**
 * Taxonomy vocabulary mappings form.
 */
function simplerdf_taxonomy_admin_vocabulary_mappings_form($form_state, $vocabulary = NULL) {
  $form['__vocabulary'] = array(
    '#type' => 'value',
    '#value' => $vocabulary,
  );

  $vid = $vocabulary->vid;
  drupal_set_title(t('Manage RDF mappings for %vocabulary', array('%vocabulary' => $vocabulary->name)));

  $form['help'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#value' => t('Enter one or more RDF property mapping (separated by commas) for each term field listed below. Namespace prefixes defined for this site are listed in the table below (open the "RDF namespaces" fieldset to view). You may use these prefixes in the property mappings. Note: specified mappings will not be checked against their vocabularies.'),
    '#suffix' => '</p>',
  );
  $form['legend'] = array(
    '#type' => 'markup',
    '#value' => theme('simplerdf_namespaces'),
  );

  // Load mappings
  $mappings = simplerdf_taxonomy_get_mappings($vid);

  // Build mappings
  $form['mappings'] = array(
    '#theme' => 'simplerdf_admin_basic_mappings_table',
  );
  $fields = simplerdf_taxonomy_get_fields($vid);
  foreach ($fields as $name => $field) {
    $mapping = array(
      '#field_title' => $field['title'],
      '#field_name' => $name,
      'properties' => array('#tree' => TRUE),
    );
    // Mapping textfield
    $mapping['properties'][$name] = array(
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 100,
      '#default_value' => implode(', ', $mappings[$name]),
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
function simplerdf_taxonomy_admin_vocabulary_mappings_form_submit(&$form, &$form_state) {
  $vid = $form_state['values']['__vocabulary']->vid;
  $values = $form_state['values'];

  $mappings = array();
  $fields = simplerdf_taxonomy_get_fields($vid);
  foreach (array_keys($fields) as $name) {
    if (!empty($values['properties'][$name])) {
      $properties = array_filter(array_map('trim', preg_split('/\s*,\s*/', $values['properties'][$name])));
      if (!empty($properties)) {
        $mappings[$name] = $properties;
      }
    }
  }
  simplerdf_taxonomy_save_mappings($vid, $mappings);

  $machine_name = _simplerdf_taxonomy_get_vocabulary_name($form_state['values']['__vocabulary']);
  simplerdf_clear_cache('mappings', 'term', $machine_name);
  simplerdf_clear_cache('term');

  drupal_set_message(t('RDF mappings have been updated.'));
}
