<?php
var_dump(extension_loaded("shared_fifo"));
var_dump(function_exists("shfifo_init"));
var_dump(function_exists("shfifo_push"));
var_dump(function_exists("shfifo_pop"));
var_dump(shfifo_init('test'));
var_dump(shfifo_push('test', "abc"));
var_dump(shfifo_push('test', "def"));
var_dump(shfifo_pop('test'));
var_dump(shfifo_pop('test'));

$sharedFifo = new SharedFifo('test2');
var_dump($sharedFifo);
var_dump($sharedFifo->push("abc"));
var_dump($sharedFifo->push("def"));
var_dump($sharedFifo->pop());
var_dump($sharedFifo->pop());