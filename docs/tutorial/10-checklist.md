# 10. Чеклист и куда дальше

Ниже — не «магический список для зачёта», а короткая проверка, что вы понимаете поведение системы под нагрузкой и при останове. Если на какой‑то пункт нет уверенного ответа, вернитесь к соответствующей главе и примеру.

## Перед сдачей работы или демо

1. **Что можно потерять, а что нельзя?**  
   Прореживаемый поток данных идёт через обычную постановку; останов, ответы и уборка — через защищённую. См. [`example01`](../../examples/example01.cpp)–[`example03`](../../examples/example03.cpp) и [главу 02](02-post-and-safe.md).

2. **Есть ли потолок у очереди, и что будет при переполнении?**  
   Вы должны уметь объяснить, появятся ли drop'ы и где вы их увидите. [`example03`](../../examples/example03.cpp), [`example04`](../../examples/example04.cpp), [глава 03](03-queue-limits.md).

3. **Не душат ли долгие задания таймеры и дедлайны?**  
   Если да — критичное вынесено в отдельный лёгкий workflow. [`example11`](../../examples/example11.cpp), [`example12`](../../examples/example12.cpp), [глава 07](07-pitfalls.md).

4. **Если заводили свой пул — его стартуют и останавливают согласованно с циклом.**  
   [`example06`](../../examples/example06.cpp), [глава 05](05-threads-and-rate.md).

5. **Отложенный код не держит голый `this` без защиты lifetime.**  
   Есть owner (`wrap` / `tracking` / `callback`) или другой явный механизм. [`example18`](../../examples/example18.cpp)–[`20`](../../examples/example20.cpp), [глава 09](09-owner.md).

6. **Число потоков не взято «с потолка».**  
   Хотя бы один замер или внятное рассуждение, почему N, а не 1. [`example09`](../../examples/example09.cpp), [`example10`](../../examples/example10.cpp).

7. **Настройки можно менять без правки логики.**  
   Внешний конфиг и/или реконфигурация на лету — плюс к зрелости решения. [`example15`](../../examples/example15.cpp), [`example08`](../../examples/example08.cpp).

## Карта примеров

| Файл | О чём пощупать руками |
|------|------------------------|
| [`example01`](../../examples/example01.cpp)–[`02`](../../examples/example02.cpp) | базовая постановка, сразу и с задержкой |
| [`example03`](../../examples/example03.cpp)–[`04`](../../examples/example04.cpp) | переполнение очереди, в том числе для отложенных |
| [`example05`](../../examples/example05.cpp), [`16`](../../examples/example16.cpp) | таймеры и события во времени |
| [`example06`](../../examples/example06.cpp)–[`07`](../../examples/example07.cpp) | пул потоков и ограничение скорости |
| [`example08`](../../examples/example08.cpp) | смена числа потоков без рестарта |
| [`example09`](../../examples/example09.cpp)–[`10`](../../examples/example10.cpp) | как threads влияет на скорость |
| [`example11`](../../examples/example11.cpp)–[`12`](../../examples/example12.cpp) | почему дедлайн опоздал и как лечить |
| [`example13`](../../examples/example13.cpp)–[`14`](../../examples/example14.cpp) | цепочки запрос–ответ |
| [`example15`](../../examples/example15.cpp) | опции из JSON |
| [`example17`](../../examples/example17.cpp) | сравнение режимов очереди |
| [`example18`](../../examples/example18.cpp)–[`20`](../../examples/example20.cpp) | owner: wrap, tracking, callback |

## Куда идти дальше

* [Оглавление](../tutorial.md)
* Точные сигнатуры — [doxygen](https://mambaru.github.io/wflow/index.html)
