//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wflow/workflow_options.hpp>
#include <string>

namespace wflow{
  
/**
 * @brief Загрузить json-конфигурацию из файла или строки.
 * @param [in] src  - имя файла или json-строка. 
 * @param [out] opt - десериализованная структура в случае успеха.
 * @param [out] err - текст ошибки, если != nullptr, и без изменений ( пустая строка ) в случае успеха.
 * @return true если загрузка и десериализация прошла успешно.
 * @details JSON допускает любые пробельные символы и переводы строк, а также многострочные комментарии в Си-стиле. 
 * Если src-не является строкой с JSON, то предполагает что это путь к файлу.
 */
bool load(const std::string& src, workflow_options* opt, std::string* err = nullptr);

/**
 * @brief Загрузить json-конфигурацию из файла или строки.
 * @param [in] src  - имя файла или json-строка. 
 * @param [out] err - текст ошибки, если != nullptr, и без изменений ( пустая строка ) в случае успеха.
 * @return десериализованная структура в случае успеха, в противном случае частично десериализация, а остальные поля оставят значения по умолчанию.
 * @details JSON допускает любые пробельные символы и переводы строк, а также многострочные комментарии в Си-стиле. 
 * Если src-не является строкой с JSON, то предполагает что это путь к файлу.
 */
workflow_options load(const std::string& src, std::string* err = nullptr); 

/**
 * @brief Сериализация в JSON структуры конфигурации.
 * @param [in] opt  - структура wflow::workflow_options, которую нужно сериализовать в JSON.
 * @return строка в формате JSON с сериализованной структурой wflow::workflow_options.
 * @details обработки ошибок не предусмотрено, т.к. сериализация всегда будет успешной.
 */
std::string dump(const workflow_options& opt);

}
