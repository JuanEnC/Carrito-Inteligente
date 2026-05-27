// ... El código original ...
// Añadir chequeo extra para la librería nipplejs
function inicializarJoystick() {
  if (!window.nipplejs) {
    console.error("Librería Nipple.js no cargada");
    return;
  }
  if (joystickManager) return;
  // ... Resto igual ...
}
// ... Resto igual ...
