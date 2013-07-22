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
      'Grants' => array(
        'class_name' => 'Grants',
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
    $this->source = new MigrateSourceCSV('/home/bc/Desktop/codez/gnl/csvs/classify_grants.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
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
    $this->source = new MigrateSourceCSV('/home/bc/Desktop/codez/gnl/csvs/classify_nonprofits_detail.csv', $this->csvcols(), array('header_rows'=>4,'embedded_newlines'=>FALSE));
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
    $this->source = new MigrateSourceCSV('/home/bc/Desktop/codez/gnl/csvs/orgs_masterlist_NTEE.csv', $this->csvcols(), array('header_rows'=>1, 'embedded_newlines'=>FALSE));
    $this->destination = new MigrateDestinationNode('org');
    $this->addFieldMapping('title', 'ORGANIZATION_NAME');
    $this->addFieldMapping(NULL, 'ORG_NTEE'); // used by prepareRow()
    //$this->addFieldMapping('field_ntee', 'ntees')
    //     ->arguments(array('source_type' => 'tid'))
    //     ->separator(',');
    $this->addFieldMapping('field_ntee', 'ntees');
    $this->addFieldMapping('field_ntee:source_type', 'tid')
         ->separator(',');


  }
  protected function csvcols() {
    $cols[1] = array('ORGANIZATION_NAME', 'org name');
    $cols[2] = array('ORG_NTEE', 'org ntees');
    return $cols;
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


class Grants extends Migration {

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import grants from a CSV');

    // Set up the source
    $this->source = new MigrateSourceCSV('/home/bc/Desktop/codez/gnl/csvs/detledger_grants_061813_Skillman_2011_IRS990_justafew.csv', array(), array('header_rows'=>1, 'embedded_newlines'=>FALSE));

    // Map fields in the source to the destination.
    // This defines the key fields for the source and the destination.
    // The first parameter names the map.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        "id" => array(
          'type' => 'varchar',
          'length' => '128',
          'not null' => TRUE,
          'description' => 'stupid fake id',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Set up grants as the destination
    $this->destination = new MigrateDestinationNode('grant');


    // destination field is the first parameter
    // and the source field is the second parameter
    $this->addFieldMapping('title', 'id'); // should be auto generated...
    $this->addFieldMapping(NULL, 'granter'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'grantee'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'amount'); // used by prepareRow()
    $this->addFieldMapping(NULL, 'year'); // used by prepareRow()
    $this->addFieldMapping('field_source', 'data_source');


    // $this->addFieldMapping('field_ntee', 'ntees');
        // ->arguments(array('source_type' => 'nid'));

    $this->addFieldMapping('field_funder', 'granter_nid');
    $this->addFieldMapping('field_recipient', 'grantee_nid');
    $this->addFieldMapping('field_amount', 'amount_clean');
    $this->addFieldMapping('field_year', 'years_clean'); // see prepareRow for date handling
  }

  public function getNidByOrgName($orgName) {
    $node = db_select('node', 'n')
              ->fields('n')
              ->condition('title', $orgName, '=')
              ->execute()
              ->fetch();
    $nid = $node->nid;
    return $nid;
  }

  public function prepareRow($row) {
    // Get the start year
    $year_raw = $row->year;
    $year_start = intval($year_raw);

    // Get the end year
    $length = intval($row->length_years);
    $year_end = $year_start + $length;

    $date_data = array(
      //'from' => $year_start . '-01-01 00:00:00',
      //'to' => $year_end . '-01-01 00:00:00',
      'from' => (string) '10/30/2011 19:43',
      'to' => (string) '12/31/2018 23:59',
    );
xdebug_break();
    $row->years_clean = drupal_json_encode($date_data);

    // Get the organziation's IDs
    $row->granter_nid = $this->getNidByOrgName($row->granter);
    $row->grantee_nid = $this->getNidByOrgName($row->grantee);
  }


}
