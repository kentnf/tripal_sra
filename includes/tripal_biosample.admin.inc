<?php
/**
 * @file
 * 
 * This file contains the functions used for the administration
 * of the biosample content type.
 * 
 * @ingroup tripal_biosample
 */

/**
 * Administrative page callbacks for the biosample content type.
 * 
 * @return
 *   A form API array describing an administrative form.
 *
 * @ingroup tripal_biosample
 */
function tripal_biosample_admin() {

  $form = array();
  
  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_biosample',
    'options' => array(
      '[biomaterial.name]' => 'BioSample Name Only ([biomaterial.name])'
    ),
    'unique_option' => '[biomaterial.name]' //*** fix  me
  );
  chado_add_admin_form_set_title($form, $form_state, $details);

  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_biosample',
    'options' => array( //*** fix these
      '/biosample/[biomaterial.biomaterial_id]' => 'BioSample ID ([biomaterial.biomaterial_id])',
      '/biosample/[biomaterial.name]' => 'BioSample Name (biomaterial.name)'
    ),
  );
  chado_add_admin_form_set_url($form, $form_state, $details);

  $form['ontologies'] = array(
    '#title' => t('biosample Ontology Terms'),
    '#type' => 'fieldset',
    '#description' => t('Configuration settings for ontology term display.'),
    '#collapsible' => TRUE,
   ); 

  $form['ontologies']['chado_biosample_ontologies_to_display'] = array(
    '#title' => t('Ontologies to Display'),
    '#description' => t('List the sequence ontologies for which cvterms should be displayed (if present)')
                      . t(' on the biosample page. Ontologies should be separated by a space or newline.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('chado_biosample_ontologies_to_display'),
  );

 return system_settings_form($form);
}

/**
 * Landing page for administration. Ensures Views are enabled and if not, provides links to do so.
 *
 * @return
 *   An HTML string that provides a link to enable the view.
 * 
 * @ingroup tripal_biosample
 */ 
function tripal_biosample_admin_biosamples_view() {
  $output = '';
  
  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('BioSamples', 'admin/tripal/extension/tripal_sra/biosample');
  drupal_set_breadcrumb($breadcrumb);

  // Add the view. 
  // $view = views_embed_view('tripal_biosample_admin_biosamples', 'default');
  $view = views_embed_view('tripal_biosample_admin_biosamples', 'default');
  if (isset($view)) {
    $output .= $view;
  }
  else { 
    $output .= '<p>The Tripal Expression module uses primarily views to provide an '
      . 'administrative interface. Currently one or more views needed for this this ' 
      . 'administrative interface are disabled. <br><strong>Click each of the following links to ' 
      . 'enable the pertinent views</strong>:</b>'; //*** fix this wording
    $output .= '<ul>';
    $output .= '<li>' . l('Biosample View', 'admin/tripal/extension/tripal_sra/content_types/biosample/views/biosamples/enable') . '</li>';
    $output .= '</ul>';
  }
  return $output;
}

