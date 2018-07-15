<pre>

<?php

//$_SERVER['DOCUMENT_ROOT'] : 절대경로를 얻는 함수
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/GameLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/Profiling_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/SystemLog_Lib.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Session_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Control.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

/*
    # DB_Log_Config 는 로그남기기 위한 데이터베이스 정보를 저장
    추가적으로 다른 데이터베이스에 연결하기 위한 정보를 저장하려면 이에 맞는 정보 셋팅 후 include
    , 해당 DB에 사용할 핸들 및 연결도 정의
*/

$GameLog;
$ProfileLog;

$JSON_Data;
$Select_Qeury;

$g_AccountNo;
$p_query_result;

/*
    1. 컨텐츠에 필요한 DB 핸들 정의 및 생성
    2. file_get_contents("php://input") 함수를 이용하여 데이터 입력 후
        explode("\r\n", ...) 수행 [ 필요에 따라 해당 DB에 쿼리문 전송 ]
    3. VersionMajor 및 VersionMinor 확인 후 json_decode 를 이용하여 데이터 분해

    #sample
        $post_string = explode("\r\n", file_get_contents("php://input"));

        $Select_Qeury = "SELECT `VersionMajor`, `VersionMinor` FROM `system`";
        $DB_Result = DB_GeneralQuery($DB_session_handle, $Select_Qeury, $p_query_result);

        $body = mysqli_fetch_array($p_query_result, MYSQLI_ASSOC);
        mysqli_free_result($p_query_result);

        if($body['VersionMajor'] === $post_string[0] && $body['VersionMinor'] === $post_string[1])
        {
            $GameLog = GameLog::GameLogInstance($config_Game_Log_URL);
            $ProfileLog = Profile::ProfileLogInstance($config_Profile_Log_URL, $_SERVER['PHP_SELF'], $config_Profile_Log_Rate);

            $aaa = strlen($post_string[3]);
            $JSON_Data = json_decode($post_string[3], true);

        }
        else
        {
            System_LOG(-1, "입력된 버전이 맞지 않음", "");
            exit;
        }
*/
?>

</pre>
