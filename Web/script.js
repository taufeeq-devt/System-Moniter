let backend;
let cpuChart, cpuGauge, memGauge;
let cpuHistory = [];
const maxPoints = 25;

new QWebChannel(qt.webChannelTransport, (channel) => {
  backend = channel.objects.backend;
  setupCharts();
  // Delay first refresh slightly to let backend initialize
  setTimeout(refreshData, 500);
  setInterval(refreshData, 2000);
});

function setupCharts() {
  const ctx = document.getElementById("cpuChart").getContext("2d");
  cpuChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: Array(maxPoints).fill(""),
      datasets: [
        {
          label: "CPU %",
          data: [],
          borderColor: "#4fc3f7",
          backgroundColor: "rgba(79,195,247,0.3)",
          borderWidth: 2,
          tension: 0.3,
          pointRadius: 0,
        },
      ],
    },
    options: {
      plugins: { legend: { labels: { color: "#fff" } } },
      scales: {
        x: { ticks: { color: "#aaa" }, grid: { color: "#333" } },
        y: { min: 0, max: 100, ticks: { color: "#aaa" }, grid: { color: "#333" } },
      },
    },
  });

  cpuGauge = new Chart(document.getElementById("cpuGauge").getContext("2d"), {
    type: "doughnut",
    data: { datasets: [{ data: [0, 100], backgroundColor: ["#4fc3f7", "#2c3e50"] }] },
    options: { cutout: "80%", plugins: { legend: { display: false } } },
  });

  memGauge = new Chart(document.getElementById("memGauge").getContext("2d"), {
    type: "doughnut",
    data: { datasets: [{ data: [0, 100], backgroundColor: ["#81d4fa", "#2c3e50"] }] },
    options: { cutout: "80%", plugins: { legend: { display: false } } },
  });
}

function refreshData() {
  try {
    // Ask backend for system data asynchronously
    backend.getSystemData(function (data) {
      if (!data || typeof data.cpu === "undefined" || typeof data.mem === "undefined") {
        console.warn("No data received yet, skipping update...");
        return;
      }

      // ✅ CPU + Memory Bars
      document.getElementById("cpuBar").style.width = data.cpu + "%";
      document.getElementById("memBar").style.width = data.mem + "%";
      document.getElementById("cpuText").textContent = data.cpu.toFixed(1) + "%";
      document.getElementById("memText").textContent = data.mem.toFixed(1) + "%";
      document.getElementById("uptimeText").textContent = data.uptime || "0h 0m";

      // ✅ CPU History Chart
      if (cpuHistory.length >= maxPoints) cpuHistory.shift();
      cpuHistory.push(data.cpu);
      cpuChart.data.datasets[0].data = cpuHistory;
      cpuChart.update();

      // ✅ Update Circular Gauges
      cpuGauge.data.datasets[0].data = [data.cpu, 100 - data.cpu];
      memGauge.data.datasets[0].data = [data.mem, 100 - data.mem];
      cpuGauge.update();
      memGauge.update();

      // ✅ Process Table
       const tbody = document.querySelector("#procTable tbody");
tbody.innerHTML = "";
(data.processes || []).slice(0, 50).forEach((p) => {
  const row = document.createElement("tr");
  row.innerHTML = `
    <td>${p.pid}</td>
    <td>${p.name}</td>
    <td>${p.cpu.toFixed(2)}</td>
    <td>${p.mem.toFixed(2)}</td>
    <td><button class="kill-btn" onclick="killProc(${p.pid})">Kill</button></td>
  `;
  tbody.appendChild(row);
});

    });
  } catch (err) {
    console.error("Error updating dashboard:", err);
  }
}


function killProc(pid) {
  if (backend) backend.killProcess(pid);
  setTimeout(refreshData, 1000);
}

function manualKill() {
  const pid = parseInt(document.getElementById("pidInput").value);
  if (!pid) {
    alert("Please enter a valid PID!");
    return;
  }
  backend.killProcess(pid);
  alert("Process " + pid + " terminated (if permissions allow).");
  document.getElementById("pidInput").value = "";
  setTimeout(refreshData, 1000);
}
