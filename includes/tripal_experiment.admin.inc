<?php
/**
 * @file
 * 
 * This file contains the functions used for the administration
 * of the experiment content type.
 * 
 * @ingroup tripal_experiment
 */

/**
 * Administrative page callbacks for the experiment content type.
 * 
 * @return
 *   A form API array describing an administrative form.
 *
 * @ingroup tripal_experiment
 */
function tripal_experiment_admin() {

  $form = array();
  
  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_experiment',
    'options' => array(
      '[experiment.name]' => 'Experiment Name Only ([experiment.name])'
    ),
    'unique_option' => '[experiment.name]' //*** fix  me
  );
  chado_add_admin_form_set_title($form, $form_state, $details);

  $details = array(
    'module' => 'tripal_sra',
    'content_type' => 'chado_experiment',
    'options' => array( //*** fix these
      '/experiment/[experiment.experiment_id]' => 'Experiment ID ([experiment.experiment_id])',
      '/experiment/[experiment.name]' => 'Experiment Name ([experiment.name])'
    ),
  );
  chado_add_admin_form_set_url($form, $form_state, $details);

  $form['ontologies'] = array(
    '#title' => t('Experiment Ontology Terms'),
    '#type' => 'fieldset',
    '#description' => t('Configuration settings for ontology term display.'),
    '#collapsible' => TRUE,
   ); 

  $form['ontologies']['chado_experiment_ontologies_to_display'] = array(
    '#title' => t('Ontologies to Display'),
    '#description' => t('List the sequence ontologies for which cvterms should be displayed (if present)')
                      . t(' on the biosample page. Ontologies should be separated by a space or newline.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('chado_experiment_ontologies_to_display'),
  );

 return system_settings_form($form);
}

/**
 * Landing page for administration. Ensures Views are enabled and if not, provides links to do so.
 *
 * @return
 *   An HTML string that provides a link to enable the view.
 * 
 * @ingroup tripal_experiment
 */ 
function tripal_experiment_admin_experiments_view() {
  $output = '';
  
  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('Experiment', 'admin/tripal/extension/tripal_sra/experiment');
  drupal_set_breadcrumb($breadcrumb);

  // Add the view. 
  $view = views_embed_view('tripal_experiment_admin_experiment', 'default');
  if (isset($view)) {
    $output .= $view;
  }
  else { 
    $output .= '<p>The Tripal Expression module uses primarily views to provide an '
      . 'administrative interface. Currently one or more views needed for this this ' 
      . 'administrative interface are disabled. <br><strong>Click each of the following links to ' 
      . 'enable the pertinent views</strong>:</b>'; //*** fix this wording
    $output .= '<ul>';
    $output .= '<li>' . l('Experiment View', 'admin/tripal/extension/tripal_sra/experiment/views/experiments/enable') . '</li>';
    $output .= '</ul>';
  }
  return $output;
}

