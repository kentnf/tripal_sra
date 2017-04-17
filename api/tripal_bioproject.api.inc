<?php
/**
 * 
 * @file
 * API for bioproject.
 * 
 * @ingroup tripal_bioproject
 */

/**
 * This function is to parse the cvterm of bioproject for display
 * 
 * @param $term
 *   The string of CV term, the sub term is connected by "_"
 * @param $num 
 *   The number of sub term need to be ignore from Head
 *
 * @return
 *   The final display of cvterm.
 *
 * @ingroup tripal_bioproject_api
 */
function tripal_bioproject_cvterm_display($term, $num) {
  $subterm = explode('_', $term);
  for($i=0; $i<$num; $i++) {
    array_shift($subterm);
  }
  return implode(' ', $subterm);
}

