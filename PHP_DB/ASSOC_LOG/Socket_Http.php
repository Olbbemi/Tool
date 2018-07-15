<pre>

<?php

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/SystemLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

function Request_Http($p_url, $p_params, $p_type = 'POST')
{
    $parsed_url = parse_url($p_url);

    if($parsed_url['scheme'] == 'http')
        $fp = fsockopen($parsed_url['host'], isset($parsed_url['port'])?$parsed_url['port']:80, $errno, $errstr, 2);
    else if($parsed_url['scheme'] == 'https')
        $fp = fsockopen("ssl://".$parsed_url['host'], isset($parsed_url['port'])?$parsed_url['port']:443, $errno, $errstr, 2);

    if($fp === false)
    {
        System_LOG(-1, "", "");
        exit;
    }

    $ContentsLength = strlen($p_params);

    $http_header = "$p_type ".$parsed_url['path']." HTTP/1.1\r\n";
    $http_header .= "Host: ".$parsed_url['host']."\r\n";
    $http_header .= "Content-Type:application/x-www-form-urlencoded\r\n";
    $http_header .= "Content-Length:".$ContentsLength."\r\n";
    $http_header .= "Connection:Close\r\n\r\n";

    if($p_type == 'POST' && isset($p_params) === true)
        $http_header .= $p_params;

    $Result = fwrite($fp, $http_header);

    // 디버그용
    $Response = fread($fp, 10000);
    echo $Response;

    fclose($fp);
    return $Result;
}

?>

</pre>
