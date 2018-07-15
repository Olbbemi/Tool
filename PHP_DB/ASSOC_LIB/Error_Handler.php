<pre>

<?php

include_once $_SERVER['DOCUMENT_ROOT']."/SystemLog_Lib.php";

// 옵션 설정
ignore_user_abort(true);
error_reporting(E_ALL);

// php 가 종료될 때 항상 호출되는 함수 [ c++ 에는 AtExit() 존재 ]
function sys_Shutdown() {}

//syntax error 을 제외한 모든 에러 및 예외 발생시 callback
set_error_handler("ERROR_Handler");
set_exception_handler("EXCEPTION_Handler"); 

function ERROR_Handler($p_errno, $p_errstr, $p_errfile, $p_errline)
{
    global $occured_accountNo;
    if(isset($occured_accountNo) === false)
        $ErrorMsg = "Errno: $p_errno FILE: $p_errfile / LINE: $p_errline / MSG: $p_errstr";

    System_LOG($occured_accountNo, "ERROR_Handler", $ErrorMsg);
}

function EXCEPTION_Handler($p_exception)
{
    System_LOG(-1, $p_exception->getMessage(), "Exception_Error");
}

 ?>

</pre>
