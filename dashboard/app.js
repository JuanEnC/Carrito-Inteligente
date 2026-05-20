// ==========================================
// 1. CONFIGURACIÓN DEL BROKER MQTT (WebSockets)
// ==========================================
// Nota el prefijo 'ws://' y el puerto 8000 para WebSockets
const brokerUrl = "ws://broker.hivemq.com:8000/mqtt";
const clientID =
  "SmartCar_WebClient_" + Math.random().toString(16).substr(2, 8);

// Tópicos configurados en tu ESP32
const topicModo = "smartcar/control/modo";

// Iniciar conexión
const client = mqtt.connect(brokerUrl, {
  clientId: clientID,
  keepalive: 60,
  clean: true,
});

// Referencia a la barra de estado visual
const statusBar = document.getElementById("status-bar");

// ==========================================
// 2. EVENTOS DE CONEXIÓN
// ==========================================
client.on("connect", () => {
  console.log("Conectado al broker MQTT vía WebSockets");
  statusBar.style.background = "#4CAF50"; // Verde
  statusBar.style.color = "white";
  statusBar.innerText = "Conectado al Carrito (Broker OK)";

  // Al conectar, forzamos al carrito a entrar en Modo Espera por seguridad
  cambiarModo(0, "Espera");

  // Forzar la velocidad media por defecto al iniciar
  cambiarVelocidad(150);
});

client.on("error", (err) => {
  console.error("Error de conexión:", err);
  statusBar.style.background = "#f44336"; // Rojo
  statusBar.innerText = "Error de Conexión MQTT";
});

client.on("close", () => {
  statusBar.style.background = "#ffcc00"; // Amarillo
  statusBar.innerText = "Desconectado. Intentando reconectar...";
});

// ==========================================
// 3. LÓGICA DE NAVEGACIÓN Y CONTROL (SPA)
// ==========================================
function cambiarModo(modoInt, nombreModo) {
  // 1. Gestionar la Interfaz Visual (Ocultar todas las vistas, mostrar la seleccionada)
  document
    .querySelectorAll(".view-container")
    .forEach((div) => div.classList.remove("active"));
  document.getElementById("vista-" + modoInt).classList.add("active");

  // Cambiar el estilo del botón activo en el menú
  document
    .querySelectorAll(".navbar button")
    .forEach((btn) => btn.classList.remove("active"));
  document.getElementById("btn-" + modoInt).classList.add("active");

  // 2. Enviar el comando al ESP32 vía MQTT
  if (client.connected) {
    // El ESP32 espera un número entero convertido a cadena de texto
    client.publish(topicModo, modoInt.toString(), { qos: 1 });
    console.log(
      `Comando enviado -> Tópico: ${topicModo} | Mensaje: ${modoInt}`,
    );
  } else {
    console.warn("No se pudo enviar el comando. No hay conexión al broker.");
    alert("Esperando conexión al broker MQTT...");
  }

  if (modoInt === 3) {
    // Inicializamos el joystick cuando se abre la vista del Modo MQTT
    inicializarJoystick();
  }

  // --- NUEVO CÓDIGO AÑADIDO PARA EL MAPA ---
  if (modoInt === 4) {
    inicializarMapa();
    // Le damos un pequeño tiempo al navegador para renderizar el div antes de redibujar el mapa
    setTimeout(() => {
      mapa.invalidateSize();
    }, 300);
  }
}

// ==========================================
// 4. CONTROL DE MOVIMIENTO (MODO 3)
// ==========================================
// Tópicos específicos para acciones
const topicMover = "smartcar/control/mover";
const topicFeedback = "smartcar/control/feedback";

// Variable de seguridad para no saturar el broker enviando la misma instrucción muchas veces seguidas
let ultimaDireccion = "";

function mover(direccion) {
  if (!client.connected) return;

  // Evita enviar el mismo comando repetidas veces si dejas presionado el botón (ahorra ancho de banda)
  if (direccion === ultimaDireccion && direccion !== "STOP") return;

  ultimaDireccion = direccion;

  // Publica el mensaje en el tópico 'smartcar/control/mover'
  client.publish(topicMover, direccion, { qos: 0 });
  console.log(`Movimiento: ${direccion}`);
}

// ==========================================
// 5. CONTROL DE LUCES Y CLAXON
// ==========================================
function enviarFeedback(accion) {
  if (!client.connected) {
    alert("Sin conexión al carrito.");
    return;
  }

  // Publica el mensaje en el tópico 'smartcar/control/feedback'
  client.publish(topicFeedback, accion, { qos: 0 });
  console.log(`Feedback: ${accion}`);
}
// ==========================================
// 6. LÓGICA DEL MAPA (MODO 4 - LEAFLET)
// ==========================================
let mapa;
let marcadorDestino;
let latitudDestino = null;
let longitudDestino = null;

// Tópico para enviar las coordenadas
const topicGPS = "smartcar/control/gps";

function inicializarMapa() {
  // Si el mapa ya existe, no lo volvemos a crear
  if (mapa) return;

  // Coordenadas iniciales: Colima, México (Lat: 19.2452, Lng: -103.7240)
  mapa = L.map("map-container").setView([19.2452, -103.724], 13);

  // Cargar los mosaicos (tiles) de OpenStreetMap
  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "© OpenStreetMap contributors",
  }).addTo(mapa);

  // Evento: Al hacer clic en el mapa, poner un marcador
  mapa.on("click", function (e) {
    latitudDestino = e.latlng.lat;
    longitudDestino = e.latlng.lng;

    // Actualizar los textos en la pantalla
    document.getElementById("target-lat").innerText = latitudDestino.toFixed(6);
    document.getElementById("target-lng").innerText =
      longitudDestino.toFixed(6);

    // Si ya hay un marcador, lo movemos; si no, lo creamos
    if (marcadorDestino) {
      marcadorDestino.setLatLng(e.latlng);
    } else {
      marcadorDestino = L.marker(e.latlng).addTo(mapa);
    }
  });
}

function enviarRutaGPS() {
  if (!client.connected) {
    alert("Sin conexión al broker MQTT.");
    return;
  }
  if (latitudDestino === null || longitudDestino === null) {
    alert("Por favor, selecciona un punto en el mapa primero.");
    return;
  }

  // Empaquetamos las coordenadas en un string, separadas por coma (ej. "19.2452,-103.7240")
  const payload = `${latitudDestino},${longitudDestino}`;

  // Publicamos en el tópico de GPS
  client.publish(topicGPS, payload, { qos: 1 });
  console.log(`Ruta GPS enviada: ${payload}`);
  alert(`Ruta enviada al carrito.\nDestino: ${payload}`);
}

// ==========================================
// 7. LÓGICA DEL JOYSTICK (NIPPLE.JS)
// ==========================================
let joystickManager;

function inicializarJoystick() {
  // Si ya existe, no lo volvemos a crear
  if (joystickManager) return;

  const zone = document.getElementById("joystick-zone");

  // Configuramos un joystick estático en el centro del contenedor
  joystickManager = nipplejs.create({
    zone: zone,
    mode: "static",
    position: { left: "50%", top: "50%" },
    color: "#2196F3",
    size: 150,
  });

  // Evento: Cuando el usuario mueve el joystick
  joystickManager.on("move", function (evt, data) {
    if (!data.angle) return; // Evita errores si no hay ángulo registrado aún

    const angle = data.angle.degree;
    let dirCmd = "STOP";

    // Mapeo matemático de los 360 grados a las 8 direcciones del ESP32
    if (angle >= 337.5 || angle < 22.5) {
      dirCmd = "DER";
    } else if (angle >= 22.5 && angle < 67.5) {
      dirCmd = "DIAG_AD_DER";
    } else if (angle >= 67.5 && angle < 112.5) {
      dirCmd = "ADELANTE";
    } else if (angle >= 112.5 && angle < 157.5) {
      dirCmd = "DIAG_AD_IZQ";
    } else if (angle >= 157.5 && angle < 202.5) {
      dirCmd = "IZQ";
    } else if (angle >= 202.5 && angle < 247.5) {
      dirCmd = "DIAG_AT_IZQ";
    } else if (angle >= 247.5 && angle < 292.5) {
      dirCmd = "ATRAS";
    } else if (angle >= 292.5 && angle < 337.5) {
      dirCmd = "DIAG_AT_DER";
    }

    // Llamamos a nuestra función existente. Ésta ya se encarga de no repetir mensajes innecesarios.
    mover(dirCmd);
    document.getElementById("joystick-status").innerText =
      "Dirección: " + dirCmd;
  });

  // Evento: Cuando el usuario suelta el joystick
  joystickManager.on("end", function (evt, data) {
    mover("STOP");
    document.getElementById("joystick-status").innerText = "Dirección: STOP";
  });
}

// ==========================================
// 8. CONTROL DE VELOCIDAD
// ==========================================
const topicVelocidad = "smartcar/control/velocidad";
let velocidadActual = 150; // Media por defecto

function cambiarVelocidad(vel) {
  if (!client.connected) return;

  velocidadActual = vel;

  // Publicamos la velocidad como un texto (ej. "150")
  client.publish(topicVelocidad, vel.toString(), { qos: 1 });
  console.log(`Velocidad cambiada a: ${vel}`);

  // Actualizamos la interfaz para que el botón activo cambie a color ámbar
  document.querySelectorAll(".speed-panel button").forEach((btn) => {
    btn.classList.remove("btn-ambar");
    btn.classList.add("btn-dark");
  });
  document.getElementById("btn-spd-" + vel).classList.remove("btn-dark");
  document.getElementById("btn-spd-" + vel).classList.add("btn-ambar");
}
