# Pruebas de TextAnalyzer sin frameworks

## Ejecutar

1. Abre `RunConfig.h` y deja `FILE_HANDLING_RUN_TESTS` en `1`.
2. Compila TextAnalyzer y ejecÃºtalo desde Visual Studio (Ctrl+F5).
3. Comprueba el resumen: `10 passed, 0 failed`.
4. Repite la ejecuciÃ³n cuando quieras; cada ejecuciÃ³n crea y elimina sus propios archivos temporales.

Para volver a la aplicaciÃ³n, desactiva todas las suites (valor `0`) y recompila.
El main ejecuta la aplicaciÃ³n cuando no hay ninguna suite seleccionada.
Cambiar una macro requiere recompilar.
Se utiliza `#if`, por lo que el valor `0` desactiva las pruebas.

El programa devuelve `0` si todas las pruebas pasan y `1` si alguna falla,
incluidos los errores de preparaciÃ³n o limpieza. Las comprobaciones siguen
activas en Release. No se necesitan archivos de entrada ni argumentos.

## AÃ±adir una prueba a FileHandling

En `FileHandlingTests.cpp`, aÃ±ade una funciÃ³n que reciba `const fs::path& directory`
y regÃ­strala en la tabla `cases`. Usa `check` para comprobar resultados y
`expectThrows` para comprobar excepciones. Cada caso recibe una carpeta vacÃ­a.
No uses rutas externas a esa carpeta. MantÃ©n los comentarios Doxygen en inglÃ©s.

Las pruebas verifican lectura y escritura de texto, sobrescritura, archivos
vacÃ­os, errores de apertura, directorios vacÃ­os o inexistentes y selecciÃ³n por
extensiÃ³n (incluida la extensiÃ³n vacÃ­a). El filtrado comprueba que no se incluyen
subdirectorios ni archivos anidados. Las listas se ordenan antes de compararlas.
No se simulan fallos de disco, permisos ni enlaces simbÃ³licos.

## AÃ±adir pruebas de otra clase

1. AÃ±ade `OtraClaseTests.h` y `OtraClaseTests.cpp` al proyecto.
2. Declara `int Tests::runOtraClaseTests()`; devuelve `0` si pasa y `1` si falla.
3. Define `OTRA_CLASE_RUN_TESTS` en `RunConfig.h`, con valor `0` o `1`.
4. Incluye su cabecera en `main.cpp` dentro de `#if OTRA_CLASE_RUN_TESTS`.
5. En el main, antes del `if (testsSelected)`, aÃ±ade:

```cpp
#if OTRA_CLASE_RUN_TESTS
    testsSelected = true;
    failures += Tests::runOtraClaseTests();
#endif
```

En el nuevo `.cpp` de pruebas incluye `../RunConfig.h` y protege la
implementaciÃ³n con su propio `#if OTRA_CLASE_RUN_TESTS`, igual que FileHandling.

El main contiene llamadas a suites. Cada suite mantiene sus casos en su propio
archivo. Para ejecutar solo una suite, activa Ãºnicamente su macro. Puedes
activar varias a la vez; el cÃ³digo de salida conserva los fallos de cualquiera.
No aÃ±adas otro main ni incluyas archivos `.cpp` desde otros archivos.

`RunConfig.h` admite tambiÃ©n una definiciÃ³n externa de la macro, gracias a
`#ifndef`; normalmente basta con editar su valor predeterminado.

## TextAnalysisSystem: prueba de integración

Activa `TEXTANALYSISSYSTEM_RUN_TESTS` en `RunConfig.h` (se deja en `1`) y
compila y ejecuta sin argumentos. Resumen esperado: `8 passed, 0 failed`.
Comprueba texto con uno y tres trabajadores, llamadas repetidas, tipos y valores
de los tres analizadores, archivos reales y vacíos, y recuperación tras un archivo
inexistente. El lote comprueba informes, orden de palabras, formato numérico,
filtrado, no recursión, sobrescritura, ausencia de coincidencias, errores de
preparación y continuidad tras un fallo de escritura. Los errores provocados
intencionadamente se muestran en stderr; no significan que el test haya fallado.
Cada caso usa su propia carpeta temporal, eliminada al terminar. No requiere
archivos del usuario. El formateador privado se comprueba mediante batchProcess.
