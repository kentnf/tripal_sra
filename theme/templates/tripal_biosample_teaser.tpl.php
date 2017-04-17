<?php
$node = $variables['node'];
$biosample = $variables['node']->biosample;
$biosample = chado_expand_var($biosample,'field','biomaterial.description'); ?>

<div class="tripal_biosample-teaser tripal-teaser">
  <div class="tripal-biosample-teaser-title tripal-teaser-title"><?php
    print l($node->title, "node/$node->nid", array('html' => TRUE));?>
  </div>
  <div class="tripal_biosample-teaser-text tripal-teaser-text"><?php
    print substr($biosample->description, 0, 650);
    if (strlen($biosample->description > 650)) {
      print "... " . l("[more]", "node/$node->nid");
    } ?>
  </div>
</div>

