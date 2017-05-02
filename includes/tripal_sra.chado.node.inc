<?php

/**
 * Implements hook_node_info().
 *
 *  Provides information to drupal about the node types that we're creating
 *  in this module. This module creates three content types:
 *  bioproject, biosample, and experiment.
 *
 * @ingroup tripal_sra
 */
function tripal_sra_node_info() {
  $nodes = array();
  $nodes['chado_bioproject'] = array(
    'name' => t('SRA BioProject'),
    'base' => 'chado_bioproject',
    'description' => t('A project from the chado database.'),
    'has_title' => TRUE,
    'locked' => TRUE,
    'chado_node_api' => array(
      'base_table' => 'project',
      'hook_prefix' => 'chado_bioproject',
      'linking_table' => 'chado_bioproject',
      'record_type_title' => array(
        'singular' => t('BioProject'),
        'plural' => t('BioProjects'),
      ),
      'sync_filters' => array(
        'type_id' => FALSE,
        'organism_id' => FALSE,
        'checkboxes' => array('name'),
      ),
    )
  );
  $nodes['chado_biosample'] = array(
    'name' => t('SRA BioSample'),
    'base' => 'chado_biosample',
    'description' => t('A BioSample base on biomaterial module of chado database.'),
    'has_title' => TRUE,
    'locked' => TRUE,
    'chado_node_api' => array(
      'base_table' => 'biomaterial',
      'hook_prefix' => 'chado_biosample',
      'linking_table' => 'chado_biosample',
      'record_type_title' => array(
        'singular' => t('BioSample'),
        'plural' => t('BioSamples'),
      ),
      'sync_filters' => array(
        'type_id' => FALSE,
        'organism_id' => FALSE,
        'checkboxes' => array('name'),
      ),
    )
  );
  $nodes['chado_experiment'] = array(
    'name' => t('SRA Experiment'),
    'base' => 'chado_experiment',
    'description' => t('A experiment from the customized chado database.'),
    'has_title' => TRUE,
    'locked' => TRUE,
    'chado_node_api' => array(
      'base_table' => 'experiment',
      'hook_prefix' => 'chado_experiment',
      'linking_table' => 'chado_experiment',
      'record_type_title' => array(
        'singular' => t('Experiment'),
        'plural' => t('Experiments'),
      ),
      'sync_filters' => array(
        'type_id' => FALSE,
        'organism_id' => FALSE,
        'checkboxes' => array('name'),
      ),
    )   
  );
  return $nodes;
}

/**
 * Implements hook_node_insert().
 * Acts on all content types.
 *
 * @ingroup tripal_sra
 */
function tripal_sra_node_insert($node) {
  switch ($node->type) {
    case 'chado_experiment':

      // Find the a experiment and add details.     
      $experiment_id = chado_get_id_from_nid('experiment', $node->nid, 'chado_experiment');
      $values = array('experiment_id' => $experiment_id);
      $experiment = chado_generate_var('experiment', $values);
      $node->experiment = $experiment;

      // Set the node title the URL path.  
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;

    case 'chado_bioproject':

      // Find the a biomaterial and add details.     
      $bioproject_id = chado_get_id_from_nid('project', $node->nid, 'chado_bioproject');
      $values = array('project_id' => $bioproject_id);
      $bioproject = chado_generate_var('project', $values);
      $node->bioproject = $bioproject;
      $node->project = $bioproject;

      // Set the node title and URL path.  
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;

    case 'chado_biosample':

      // Find the a biomaterial and add details.     
      $biosample_id = chado_get_id_from_nid('biomaterial', $node->nid, 'chado_biosample');
      $values = array('biomaterial_id' => $biosample_id);
      $biosample = chado_generate_var('biomaterial', $values);
      $node->biosample = $biosample;
      $node->biomaterial = $biosample;

      // Set the node title and URL path.  
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;
  }
}

/**
 * Implements hook_node_update
 *
 * @ingroup tripal_sra
 */
function tripal_sra_node_update($node) {

  switch ($node->type) {
    case 'chado_experiment':
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;

    case 'chado_biosample':
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;

    case 'chado_bioproject':
      $node->title = chado_get_node_title($node);
      chado_set_node_url($node);
      break;
  }
}

/**
 * Implements hook_node_presave().
 *
 * @ingroup tripal_sra
 */
function tripal_sra_node_presave($node) {
  switch ($node->type) {
    case 'chado_experiment':
      if (property_exists($node, 'experiment_name')) {
        $node->title = $node->experiment_name;
      }
      else {
        if (property_exists($node, 'experiment')) {
          $node->title = $node->arraydesign->name;
        }
      }
      break;

    case 'chado_bioproject':
      if (property_exists($node, 'bioproject_name')) {
        $node->title = $node->bioproject_name;
      }
      else {
        if (property_exists($node, 'project')) {
          $node->title = $node->project->name;
        }
        if (property_exists($node, 'bioproject')) {
          $node->title = $node->bioproject->name;
        }
      }
      break;

    case 'chado_biomsample':
      if (property_exists($node, 'biosample_name')) {
        $node->title = $node->biosample_name;
      }
      else {
        if (property_exists($node, 'biomaterial')) {
          $node->title = $node->biomaterial->name;
        }
        if (property_exists($node, 'biosample')) {
          $node->title = $node->biosample->name;
        }
      }
      break;
  }
}

/**
 * Implements hook_node_view().
 *
 * @ingroup tripal_sra
 */
function tripal_sra_node_view($node, $view_mode, $langcode) {
  switch ($node->type) {
    case 'chado_bioproject':
      if ($view_mode == 'full') {
        $node->content['tripal_bioproject_base'] = array(
          '#markup' => theme('tripal_bioproject_base', array('node' => $node)),
          '#tripal_toc_id' => 'base',
          '#tripal_toc_title' => 'Overview',
          '#weight' => -100,
        );
        $node->content['tripal_bioproject_experiments'] = array(
          '#markup' => theme('tripal_bioproject_experiments', array('node' => $node)),
          '#tripal_toc_id' => 'experiments',
          '#tripal_toc_title' => 'Experiments',
        );
      }
      if ($view_mode == 'teaser') {
        $node->content['tripal_bioproject_teaser'] = array(
          '#markup' => theme('tripal_bioproject_teaser', array('node' => $node)),
        );
      }
      break;

    case 'chado_biosample':
      if ($view_mode == 'full') {
        $node->content['tripal_biosample_base'] = array(
          '#markup' => theme('tripal_biosample_base', array('node' => $node)),
          '#tripal_toc_id' => 'base',
          '#tripal_toc_title' => 'Overview',
          '#weight' => -100,
        );
      }
      if ($view_mode == 'teaser') {
        $node->content['tripal_biosample_teaser'] = array(
          '#markup' => theme('tripal_biosample_teaser', array('node' => $node)),
        );
      }
      break;

    case 'chado_experiment':
      if ($view_mode == 'full') {
        $node->content['tripal_experiment_base'] = array(
          '#markup' => theme('tripal_experiment_base', array('node' => $node)),
          '#tripal_toc_id' => 'base',
          '#tripal_toc_title' => 'Overview',
          '#weight' => -100,
        );
      }
      if ($view_mode == 'teaser') {
        $node->content['tripal_experiment_teaser'] = array(
          '#markup' => theme('tripal_experiment_teaser', array('node' => $node)),
        );
      }
      break;
  }
}

