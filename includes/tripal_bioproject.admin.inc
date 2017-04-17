<?php
/**
 * @file
 * 
 * This file contains the functions used for the administration
 * of the bioproject content type.
 * 
 * @ingroup tripal_bioproject
 */

/**
 * Administrative page callbacks for the bioproject content type.
 * 
 * @return
 *   A form API array describing an administrative form.
 *
 * @ingroup tripal_bioproject
 */
function tripal_bioproject_admin() {

  $form = array();
  
  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_bioproject',
    'options' => array(
      '[project.name]' => 'BioProject Name Only'
    ),
    'unique_option' => '[project.name]' //*** fix  me
  );
  chado_add_admin_form_set_title($form, $form_state, $details);

  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_bioproject',
    'options' => array( //*** fix these
      '/bioproject/[project.project_id]' => 'BioProject ID',
    ),
  );
  chado_add_admin_form_set_url($form, $form_state, $details);

  $form['ontologies'] = array(
    '#title' => t('BioProject Ontology Terms'),
    '#type' => 'fieldset',
    '#description' => t('Configuration settings for ontology term display.'),
    '#collapsible' => TRUE,
   ); 

  $form['ontologies']['chado_bioproject_ontologies_to_display'] = array(
    '#title' => t('Ontologies to Display'),
    '#description' => t('List the sequence ontologies for which cvterms should be displayed (if present)')
                      . t(' on the BioProject page. Ontologies should be separated by a space or newline.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('chado_bioproject_ontologies_to_display'),
  );

 return system_settings_form($form);
}

/**
 * Landing page for administration. Ensures Views are enabled and if not, provides links to do so.
 *
 * @return
 *   An HTML string that provides a link to enable the view.
 * 
 * @ingroup tripal_project
 */ 
function tripal_bioproject_admin_bioprojects_view() {
  $output = '';
  
  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('BioProject', 'admin/tripal/extension/tripal_sra/bioproject');
  drupal_set_breadcrumb($breadcrumb);

  // Add the view. 
  $view = views_embed_view('tripal_bioproject_admin_bioprojects', 'default');
  if (isset($view)) {
    $output .= $view;
  }
  else { 
    $output .= '<p>The Tripal Expression module uses primarily views to provide an '
      . 'administrative interface. Currently one or more views needed for this this ' 
      . 'administrative interface are disabled. <br><strong>Click each of the following links to ' 
      . 'enable the pertinent views</strong>:</b>'; //*** fix this wording
    $output .= '<ul>';
    $output .= '<li>' . l('BioProject View', 'admin/tripal/extension/tripal_sra/bioproject/views/bioprojects/enable') . '</li>';
    $output .= '</ul>';
  }
  return $output;
}

