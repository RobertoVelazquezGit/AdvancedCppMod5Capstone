# Pruebas de TextAnalyzer sin frameworks

## Ejecutar

1. Abre `RunConfig.h` y deja `FILE_HANDLING_RUN_TESTS` en `1`.
2. Compila TextAnalyzer y ejecútalo desde Visual Studio (Ctrl+F5).
3. Comprueba el resumen: `10 passed, 0 failed`.
4. Repite la ejecución cuando quieras; cada ejecución crea y elimina sus propios archivos temporales.

Para volver a la aplicación, desactiva todas las suites (valor `0`) y recompila.
El main ejecuta la aplicación cuando no hay ninguna suite seleccionada.
Cambiar una macro requiere recompilar.
Se utiliza `#if`, por lo que el valor `0` desactiva las pruebas.

El programa devuelve `0` si todas las pruebas pasan y `1` si alguna falla,
incluidos los errores de preparación o limpieza. Las comprobaciones siguen
activas en Release. No se necesitan archivos de entrada ni argumentos.

## Añadir una prueba a FileHandling

En `FileHandlingTests.cpp`, añade una función que reciba `const fs::path& directory`
y regístrala en la tabla `cases`. Usa `check` para comprobar resultados y
`expectThrows` para comprobar excepciones. Cada caso recibe una carpeta vacía.
No uses rutas externas a esa carpeta. Mantén los comentarios Doxygen en inglés.

Las pruebas verifican lectura y escritura de texto, sobrescritura, archivos
vacíos, errores de apertura, directorios vacíos o inexistentes y selección por
extensión (incluida la extensión vacía). El filtrado comprueba que no se incluyen
subdirectorios ni archivos anidados. Las listas se ordenan antes de compararlas.
No se simulan fallos de disco, permisos ni enlaces simbólicos.

## Añadir pruebas de otra clase

1. Añade `OtraClaseTests.h` y `OtraClaseTests.cpp` al proyecto.
2. Declara `int Tests::runOtraClaseTests()`; devuelve `0` si pasa y `1` si falla.
3. Define `OTRA_CLASE_RUN_TESTS` en `RunConfig.h`, con valor `0` o `1`.
4. Incluye su cabecera en `main.cpp` dentro de `#if OTRA_CLASE_RUN_TESTS`.
5. En el main, antes del `if (testsSelected)`, añade:

```cpp
#if OTRA_CLASE_RUN_TESTS
    testsSelected = true;
    failures += Tests::runOtraClaseTests();
#endif
```

En el nuevo `.cpp` de pruebas incluye `../RunConfig.h` y protege la
implementación con su propio `#if OTRA_CLASE_RUN_TESTS`, igual que FileHandling.

El main contiene llamadas a suites. Cada suite mantiene sus casos en su propio
archivo. Para ejecutar solo una suite, activa únicamente su macro. Puedes
activar varias a la vez; el código de salida conserva los fallos de cualquiera.
No añadas otro main ni incluyas archivos `.cpp` desde otros archivos.

`RunConfig.h` admite también una definición externa de la macro, gracias a
`#ifndef`; normalmente basta con editar su valor predeterminado.
