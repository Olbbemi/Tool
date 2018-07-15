<pre>

<?php

/*
#SystemLog 테이블 템플릿

CREATE TABLE `SystemLog_template`
(
	  `Index`			int(11) NOT NULL AUTO_INCREMENT,
	  `Date`			datetime NOT NULL,
	  `AccountNo`	varchar(50) NOT NULL,
	  `Action`		varchar(128) DEFAULT NULL,
	  `Message`		varchar(1024) DEFAULT NULL,
	   PRIMARY KEY (`Index`)
) ENGINE=MyISAM;
*/

//------------------------------------------------------------
// 시스템 로그 남기는 함수 / DB 및 테이블 정보는 DB_Config.php 참고.
//
// 문자열 인자의 공격 검사는 하지 않음, 내부 서버 IP 외에는 본 파일을 호출하지 못하도록 방화벽에서 차단 되어야 함
// POST 방식으로 로그 스트링을 저장한다.
//
// $_POST['AccountNo']	: 유저
// $_POST['Action']		: 액션구분
// $_POST['Message']	: 로그스트링
//------------------------------------------------------------

include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Log_Config.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_DB/DB_Control.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

$errno = 0; // DB_TransactionQuery 함수에 사용되는 out 변수로 해당 함수내부에서 에러 발생시 확인할 변수
$recent_AI_id; // DB_TransactionQuery 함수에서 사용되는 out 변수로 해당 세션의 최근 AI를 확인할 변수
$Trans_Query = array();

$JSON_Data = json_decode(file_get_contents("php://input"), true);
$AccountNo = $JSON_Data['AccountNo'];
$Action    = $JSON_Data['Action'];
$Message   = $JSON_Data['Message'];

$check = DB_Connection($config_log_DB_system_handle, $config_log_DB_host, $config_log_DB_id, $config_log_DB_pass, $config_log_DB_name, $config_log_DB_port);
if($check === false)
{
    System_LOG(-1, "", "");
    exit;
}

// 현재날짜 테이블에 로그 삽입
$Table_Name = "SystemLog_".date("Ym");
$Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `AccountNo`, `Action`, `Message`) VALUES (NULL, NOW(), '{$AccountNo}', '{$Action}', '{$Message}')";

$Result = DB_TransactionQuery($config_log_DB_system_handle, $Trans_Query, $recent_AI_id, $errno);

if ($Result === false)
{
		// 해당 테이블이 존재하지 않는 경우 생성 후 입력
		if($errno === 1146)
		{
				unset($Trans_Query);
				$Trans_Query[] = "CREATE TABLE `{$Table_Name}` LIKE `systemlog_template`";
				$Trans_Query[] = "INSERT INTO `{$Table_Name}` (`Index`, `Date`, `AccountNo`, `Action`, `Message`) VALUES (NULL, NOW(), '{$AccountNo}', '{$Action}', '{$Message}')";
				DB_TransactionQuery($config_log_DB_system_handle, $Trans_Query, $recent_AI_id, $errno);
		}
		else
				System_LOG(-1, "Create Table Fail", "errno : '{$errno}'");
}

DB_Disconnection($DB_handle);

?>

</pre>
