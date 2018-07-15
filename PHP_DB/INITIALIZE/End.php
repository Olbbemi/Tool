<pre>

<?php

/*
    1. 컨텐츠에서 저장한 GameLog 및 ProfileLog 를 한번에 저장
    2. 컨텐츠에서 사용한 DB 연결 종료
*/

$GameLog->SaveLog();
$ProfileLog->LOG_Save($g_AccountNo);

 ?>

</pre>
