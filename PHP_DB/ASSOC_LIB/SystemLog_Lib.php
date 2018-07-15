<pre>

<?php

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Socket_Http.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

/*
    시스템에 관련된 로그를 남기고 싶을 때 해당 함수를 호출하면 됨
    config_Log_Level 및 config_System_Log_URL 은 Log_Config.php 에서 수정
*/
function System_LOG($p_AccountNo, $p_Action, $p_Message, $p_Level = 1)
{
    // $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Log_Config.php 에 존재하는 변수
    global $config_Log_Level;
    global $config_System_Log_URL;

    if($config_Log_Level < $p_Level)
        return;

    if($p_AccountNo < 0)
    {
        if(array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER))
            $p_AccountNo = $_SERVER['HTTP_X_FORWARDED_FOR'];
        else if(array_key_exists('REMOTE_ADDR',$_SERVER))
            $p_AccountNo = $_SERVER["REMOTE_ADDR"];
        else
            $p_AccountNo = 'local';
    }

    $post_string = array("AccountNo" => $p_AccountNo, "Action" => $p_Action, "Message" => $p_Message);
    Request_Http($config_System_Log_URL, json_encode($post_string), "POST");
}

 ?>

</pre>
