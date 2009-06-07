<?php
/*
 * SimpleID
 *
 * Copyright (C) Kelvin Mo 2007-9
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * $Id$
 */

/**
 * Functions for persistent storage via the file system.
 *
 * In general, there are three different sets of data which SimpleID needs
 * to store:
 *
 * - transient data (e.g. OpenID associations, sessions, auto-login)
 * - application data (e.g. salt for form tokens)
 * - user data (e.g. user names, passwords and settings)
 *
 * Prior to version 0.7, both transient data and application data are stored
 * using {@link cache.inc}.  From version 0.7, application data are now
 * stored separately from the cache.
 *
 * Prior to version 0.7, user data is only stored in the identity file, to which
 * SimpleID cannot write.  This means that if the user wishes to change a setting,
 * he or she will need to edit the identity file manually.  Other user settings
 * (e.g. RP preferences) are stored using {@link cache.inc}
 *
 * From version 0.7, user data is stored in two files, one is the identity
 * file, the other is the user store file, which SimpleID can write.
 *
 * @package simpleid
 * @filesource
 */

/**
 * This variable is a cache of SimpleID's application settings.  It is populated
 * progressively as {@link store_get()} is called.
 *
 * @global array $settings
 */
$settings = array();

/**
 * Returns whether the user name exists in the user store.
 *
 * @param string $uid the name of the user to check
 * @param bool whether the user name exists
 */
function store_user_exists($uid) {
    $identity_file = SIMPLEID_IDENTITIES_DIR . "/$uid.identity";
    return (file_exists($identity_file));
}

/**
 * Loads user data for a specified user name.
 *
 * The user name must exist.  You should check whether the user name exists with
 * the {@link store_user_exists()} function
 *
 * @param string $uid the name of the user to load
 * @return mixed data for the specified user
 */
function store_user_load($uid) {
    $store_file = SIMPLEID_STORE_DIR . "/$uid.usrstore";
    
    if (file_exists($store_file)) {
        $data = unserialize(file_get_contents($store_file));
    } else {
        $data = array();
    }
    
    $identity_file = SIMPLEID_IDENTITIES_DIR . "/$uid.identity";
    $data = array_merge($data, parse_ini_file($identity_file, TRUE));
    
    return $data;
}

/**
 * Finds the user name from a specified OpenID Identity URI.
 *
 * @param string $identity the Identity URI of the user to load
 * @return string the user name for the Identity URI, or NULL if no user has
 * the specified Identity URI
 */
function store_get_uid($identity) {
    $uid = cache_get('identity', $identity);
    if ($uid !== NULL) return $uid;
    
    $r = NULL;
    
    $dir = opendir(SIMPLEID_IDENTITIES_DIR);
    
    while (($file = readdir($dir)) !== false) {
        $filename = SIMPLEID_IDENTITIES_DIR . '/' . $file;
        
        if ((filetype($filename) != "file") || (!preg_match('/^(.+)\.identity$/', $file, $matches))) continue;
        
        $uid = $matches[1];
        $test_user = store_user_load($uid);
        
        cache_set('identity', $test_user['identity'], $uid);
    
        if ($test_user['identity'] == $identity) {
            $r = $uid;
        }
    }
        
    closedir($dir);
    
    return $r;
}

/**
 * Saves user data for a specific user name.
 *
 * This data is stored in the user store file.
 *
 * @param string $uid the name of the user
 * @param array $data the data to save
 * @param array $exclude an array of keys to exclude from the user store file.
 * These are generally keys which are stored in the identity file.
 *
 * @since 0.7
 */
function store_user_save($uid, $data, $exclude = array()) {
    foreach ($exclude as $key) {
        if (isset($data[$key])) unset($data[$key]);
    }
    
    $store_file = SIMPLEID_STORE_DIR . "/$uid.usrstore";
    $file = fopen($store_file, 'w');
    fwrite($file, serialize($data));
    fclose($file);
}

/**
 * Loads an application setting.
 *
 * @param string $name the name of the setting to return
 * @param mixed $default the default value to use if this variable has never been set
 * @return mixed the value of the setting
 *
 */
function store_get($name, $default = NULL) {
    global $settings;
    
    if (!isset($settings[$name])) {
        $setting_file = SIMPLEID_STORE_DIR . "/$name.setting";
        
        if (file_exists($setting_file)) {
            $settings[$name] = unserialize(file_get_contents($setting_file));
        } else {
            return $default;
        }
    }
    
    return $settings[$name];
}

/**
 * Saves an application setting.
 *
 * @param string $name the name of the setting to save
 * @param mixed $value the value of the setting
 *
 */
function store_set($name, $value) {
    global $settings;
    
    $settings[$name] = $value;
    
    $setting_file = SIMPLEID_STORE_DIR . "/$name.setting";
    $file = fopen($store_file, 'w');
    fwrite($file, serialize($data));
    fclose($file);
}

/**
 * Deletes an application setting.
 *
 * @param string $name the name of the setting to delete
 *
 */
function store_del($name) {
    global $settings;
    
    if (isset($settings[$name])) unset($settings[$name]);;
    
    $setting_file = SIMPLEID_STORE_DIR . "/$name.setting";
    if (file_exists($setting_file)) unlink($setting_file);
}
?>