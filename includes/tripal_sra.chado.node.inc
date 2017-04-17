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

