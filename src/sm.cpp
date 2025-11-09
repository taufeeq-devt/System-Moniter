#include <QApplication>
#include <QWebEngineView>
#include <QWebChannel>
#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QDir>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <signal.h>

struct Process {
    int pid;
    std::string name;
    double cpuSeconds;
    double memMB;
};

// -------------------- System CPU Usage --------------------
float getCPUUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    static long prevTotal = 0, prevIdle = 0;
    long total = user + nice + system + idle + iowait + irq + softirq + steal;
    float diffTotal = total - prevTotal;
    float diffIdle = idle - prevIdle;
    prevTotal = total; prevIdle = idle;
    if (diffTotal == 0) return 0;
    return (1.0 - diffIdle / diffTotal) * 100.0;
}

// -------------------- Memory Usage (%) --------------------
float getMemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string key; long total = 0, avail = 0;
    while (file >> key) {
        if (key == "MemTotal:") file >> total;
        else if (key == "MemAvailable:") file >> avail;
    }
    return (float)(total - avail) / total * 100.0;
}

// -------------------- System Uptime --------------------
std::string getUptime() {
    struct sysinfo info;
    sysinfo(&info);
    int hours = info.uptime / 3600;
    int mins = (info.uptime % 3600) / 60;
    return std::to_string(hours) + "h " + std::to_string(mins) + "m";
}

// -------------------- Process List with CPU + MEM --------------------
std::vector<Process> getProcesses() {
    std::vector<Process> procs;
    DIR* dir = opendir("/proc");
    if (!dir) return procs;

    struct dirent* ent;
    long pageSizeKB = sysconf(_SC_PAGESIZE) / 1024; // convert page size to KB
    long clockTicks = sysconf(_SC_CLK_TCK);

    while ((ent = readdir(dir)) != NULL) {
        if (!isdigit(ent->d_name[0])) continue;
        int pid = atoi(ent->d_name);

        std::ifstream cmd("/proc/" + std::to_string(pid) + "/comm");
        std::string name;
        if (!std::getline(cmd, name)) continue;

        // Read CPU times
        std::ifstream statf("/proc/" + std::to_string(pid) + "/stat");
        double utime = 0, stime = 0;
        if (statf.is_open()) {
            std::string temp;
            int idx = 0;
            while (statf >> temp) {
                ++idx;
                if (idx == 14) utime = std::stod(temp);
                if (idx == 15) { stime = std::stod(temp); break; }
            }
        }
        double cpuSeconds = (utime + stime) / clockTicks;

        // Read memory (RSS)
        std::ifstream status("/proc/" + std::to_string(pid) + "/status");
        double memKB = 0;
        std::string line;
        while (std::getline(status, line)) {
            if (line.rfind("VmRSS:", 0) == 0) {
                std::istringstream iss(line);
                std::string label;
                iss >> label >> memKB;
                break;
            }
        }
        double memMB = memKB / 1024.0;

        procs.push_back({pid, name, cpuSeconds, memMB});
    }
    closedir(dir);

    // Sort by CPU time descending
    std::sort(procs.begin(), procs.end(),
              [](const Process &a, const Process &b) { return a.cpuSeconds > b.cpuSeconds; });

    return procs;
}

// -------------------- Monitor Class --------------------
class Monitor : public QObject {
    Q_OBJECT
public slots:
    QVariantMap getSystemData() {
        QVariantMap data;
        data["cpu"] = getCPUUsage();
        data["mem"] = getMemoryUsage();
        data["uptime"] = QString::fromStdString(getUptime());

        QVariantList plist;
        for (auto &p : getProcesses()) {
            QVariantMap m;
            m["pid"] = p.pid;
            m["name"] = QString::fromStdString(p.name);
            m["cpu"] = p.cpuSeconds;
            m["mem"] = p.memMB;
            plist.append(m);
        }
        data["processes"] = plist;
        return data;
    }

    void killProcess(int pid) {
        ::kill(pid, SIGKILL);
    }
};

// -------------------- Main Function --------------------
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWebEngineView view;
    QWebChannel channel;
    Monitor monitor;

    channel.registerObject(QStringLiteral("backend"), &monitor);
    view.page()->setWebChannel(&channel);
    view.load(QUrl::fromLocalFile(QDir::current().filePath("dashboard.html")));

    view.resize(1100, 800);
    view.show();
    return app.exec();
}

// Must be at end for Qt MOC
#include "sm.moc"
