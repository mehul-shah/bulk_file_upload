<?php

/**
 * Call To Bulk File Upload Page
 */

function bulk_file_upload() {
  $output = '';
  $form = drupal_get_form('bulk_file_upload_form');
  $output .= render($form);
  return $output;//returns output
}

/**
 * Call To Bulk File Upload Form
 */

function bulk_file_upload_form($form, &$form_state) {
  $temp = bulk_file_upload_get_directory();
  $form['path'] = array(
    '#type' => 'select',
    '#title' => t('Select File Upload Location'),
    '#options' => $temp,
  );
  $extension = variable_get('bulk_file_upload_allowed_file_extension','');
  $extension = implode(',',$extension);
  $form['file_extension'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed File Extension'),
    '#size' => 96,
    '#description' => t('Use comma to separate multiple extensstion'),
    '#default_value' => $extension,
  );
  $setting_extension = variable_get('bulk_file_upload_allowed_file_extension','');
  $setting_extension = implode(',',$setting_extension);
  $form['zip_file'] = array(
    '#type' => 'file',
    '#title' => t('Upload Zip File'),
    '#size' => 48,
    '#description' => t('Warning : MAX SIZE LIMIT : 30MB<br/>ZIP File should contain Files directly, not in Folder<br/>Image Extesions allowed are '.$setting_extension),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Upload',
  );
  return $form;
}

/**
 * Call To Form_validate
 */

function bulk_file_upload_form_validate($form, &$form_state) {
  $setting_extension = explode(',',str_replace(' ','',$form_state['values']['file_extension']));
  variable_set('bulk_file_upload_allowed_file_extension',$setting_extension);
  $selected_path = $form_state['values']['path'];
  global $base_url;
  $path_check = $base_url.'/sites/default/files/'.$selected_path;
  $file = file_save_upload('zip_file', array('file_validate_extensions' => array('zip'),),'public://'.$selected_path,FILE_EXISTS_REPLACE);
  $zip = new ZipArchive;
  $zip = zip_open(drupal_realpath($file->uri));
  $count = 0;
  $invalid_files = '';
  if(is_resource($zip)) {
    while ($zip_entry = zip_read($zip)) {
      $file_name = zip_entry_name($zip_entry);
      $extension = explode(".", $file_name);
      $extension = end($extension);
      $setting_extension = variable_get('bulk_file_upload_allowed_file_extension','');
      if (!in_array($extension,$setting_extension)) {
        $count++;
        $invalid_files .= $count.')'.$file_name.'<br/>';
      }
    }
    $zip = zip_close($zip);
    if ($count > 0) {
      drupal_set_message(t("An error occurred and processing did not complete.<br/>Please Upload only image files.<br/>There are $count invalid files as<br/>$invalid_files"), 'error');
    }
    else {
      $zip = new ZipArchive;
      $res = $zip->open(drupal_realpath($file->uri));
      $zip->extractTo('./sites/default/files/'.$selected_path);
      $zip->close();
      drupal_set_message(t("File Uploaded Successfully<br/> You can check uploaded image using $path_check + image name"), 'status');
      unlink(drupal_realpath($file->uri));
    }
  }
}

/**
 * Function To locate available directory in public folder
 */

function bulk_file_upload_get_directory(){
  $folders = array();
  if ($handle = opendir('./sites/default/files/')) {
    while (false !== ($entry = readdir($handle))) {
      if ($entry != "." && $entry != ".." && is_dir('./sites/default/files/'.$entry)) {
        $folders[$entry] = $entry;
      }
    }
    closedir($handle);
  }
  return $folders;
}