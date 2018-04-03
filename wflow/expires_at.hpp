#pragma once 

namespace wflow {

/** 
 * @enum wflow::expires_at
 * @brief  Флаги для таймеров - с какого момента производить отсчет времени   
 */  
enum class expires_at
{
  before = 1, /**< Начинать отсчет до вызова обработчика таймера  */
  after  = 2  /**< Начинать отсчет после вызова обработчика таймера  */
};

}
