<?php

/**
 * @file migrate_gnl.migrate.inc
 * Migrate content from CSVs into detroit ledger taxonomy terms
 */

/**
 * Implements hook_migrate_api().
 */
function migrate_gnl_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'terms' => array(
        'title' => t('Terms'),
      ),
      'nodes' => array(
        'title' => t('Nodes'),
      ),
    ),
    'migrations' => array(
      'GrantTypes' => array(
        'class_name' => 'GrantTypes',
        'group_name' => 'terms',
      ),
      'OrgNTEETypes' => array(
        'class_name' => 'OrgNTEETypes',
        'group_name' => 'terms',
      ),
      'Orgs' => array(
        'class_name' => 'Orgs',
        'group_name' => 'nodes',
      ),
    ),
  );
  return $api;
}

class GrantTypes extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import grant types from CSV.');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "type" => array(
          'type' => 'varchar',
          'length' => '255',
          'not null' => TRUE,
          'description' => 'grant type',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/data/disk/gnl/csvs/classify_grants.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('grant_types');
    $this->addFieldMapping('name', 'type');
    $this->addFieldMapping('description', 'description');
  }
  protected function csvcols() {
    $cols[0] = array('type', 'grant type');
    $cols[1] = array('description', 'description');
    return $cols;
  }
}

class OrgNTEETypes extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import organization NTEE codes and descriptions from CSV into term name and ntee code fields.');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "code" => array(
          'type' => 'varchar',
          'length' => '8',
          'not null' => TRUE,
          'description' => 'ntee code',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/data/disk/gnl/csvs/classify_nonprofits_detail.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationTerm('ntee');
    $this->addFieldMapping('name', 'description');
    $this->addFieldMapping('field_ntee_code', 'code');
  }
  protected function csvcols() {
    $cols[0] = array('code', 'ntee code');
    $cols[1] = array('description', 'description');
    return $cols;
  }
}

class Orgs extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import organizations from a CSV like orgs_masterlist_NTEE.csv');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "ORGANIZATION_NAME" => array(
          'type' => 'varchar',
          'length' => '128',
          'not null' => TRUE,
          'description' => 'org name',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
    $this->source = new MigrateSourceCSV('/data/disk/gnl/csvs/orgs_masterlist_NTEE.csv', $this->csvcols(), array('header_rows'=>1, 'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationNode('org');
    $this->addFieldMapping('title', 'ORGANIZATION_NAME');
    $this->addFieldMapping(NULL, 'ORG_NTEE'); // used by prepareRow()
    $this->addFieldMapping('field_ntee', 'ntees')
         ->arguments(array('source_type' => 'tid'))
         ->separator(',');
  }
  protected function csvcols() {
    $columns[1] = array('ORGANIZATION_NAME', 'org name');
    $columns[2] = array('ORG_NTEE', 'org ntees');
  }
  public function prepareRow($row) {
    $codes = explode(';', $row->ORG_NTEE);
    $tids = array();
    foreach ($codes as $code) {
      $tids[] = db_select('field_data_field_ntee_code', 'ntee')
                  ->fields('ntee', array('entity_id'))
                  ->condition('field_ntee_code_value', $code)
                  ->execute()
                  ->fetchField();
    }
    $tids = implode(',', $tids);
    $row->ntees = $tids;
  }
  public function fields() {
    return array(
      'title' => 'Organization name',
      'ntees' => 'Comma-separated list of NTEE tids',
    );
  }
}

