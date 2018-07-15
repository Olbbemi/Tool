<pre>

<?php
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LOG/Socket_Http.php";
include_once $_SERVER['DOCUMENT_ROOT']."/ASSOC_LIB/ERROR_Handler.php";

class GameLog
{
    private $LOG_URL = '';
    private $Log_Array = array();

    static function GameLogInstance($p_game_URL)
    {
        static $game_instance;
        if(isset($game_instance) === false)
            $game_instance = new GameLog();

        $game_instance->LOG_URL = $p_game_URL;
        return $game_instance;
    }

    // 상황에 따라 저장할 매개변수는 변할 수 있음
    function AddLog($p_AccountNo, $p_Type, $p_Code, $p_Param1 = 0, $p_Param2 = 0, $p_Param3 = 0, $p_Param4 = 0, $p_ParamString = '')
    {
        $this->Log_Array[] = array( "AccountNo"    =>  $p_AccountNo,
                                    "LogType"      =>  $p_Type,
                                    "LogCode"      =>  $p_Code,
                                    "Param1"       =>  $p_Param1,
                                    "Param2"       =>  $p_Param2,
                                    "Param3"       =>  $p_Param3,
                                    "Param4"       =>  $p_Param4,
                                    "ParamString"  =>  $p_ParamString );
    }

    function SaveLog()
    {
        if(0 < count($this->Log_Array))
            Request_Http($this->LOG_URL, json_encode($this->Log_Array), "POST");
    }
}

 ?>

</pre>
