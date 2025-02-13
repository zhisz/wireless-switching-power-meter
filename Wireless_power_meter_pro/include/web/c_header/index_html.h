//该文件由embed_files.py自动生成,请勿手动修改
#ifndef index_html_H
#define index_html_H
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="zh">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>实时电压电流监测</title>
  <script src="/jquery-3.7.1.min.js"></script>
  <script src="/highcharts.js"></script>
  <style>
    /* 简单样式，适配手机端 */
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      /* 垂直居中 */
      height: 100vh;
      /* 页面高度 */
      background-color: #f4f4f4;
    }

    .container {
      width: 100%;
      max-width: 800px;
      margin: 20px;
      background-color: #fff;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    .real-time-data {
      display: flex;
      flex-direction: column;
      /* 每个数据为一行 */
      margin-bottom: 20px;
      width: 100%;
    }

    .real-time-data div {
      font-size: 24px;
      /* 增大字体大小 */
      font-weight: bold;
      margin-bottom: 10px;
      /* 每行之间的间距 */
    }

    .chart-container {
      height: 400px;
      margin-bottom: 20px;
      width: 100%;
    }

    /* 圆形开关样式 */
    .switch {
      position: relative;
      display: inline-block;
      width: 20vh;
      height: 20vh;
      /* 高度为页面高度的20% */
      max-height: 100px;
      /* 最大高度限制 */
      margin: 20px 0;
      /* 上下留出间距 */
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      transition: 0.2s;
      border-radius: 50vh;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: calc(100% - 8px);
      /* 滑块高度 */
      width: calc(50% - 4px);
      /* 滑块宽度 */
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: 0.2s;
      border-radius: 50%;
    }

    input:checked+.slider {
      background-color: #4CAF50;
    }

    input:checked+.slider:before {
      transform: translateX(calc(100% - 8px));
      /* 滑块滑动距离 */
    }

    .slider.off {
      background-color: #f44336;
    }
  </style>
</head>

<body>
  <div class="container">
    <!-- 实时数据展示 -->
    <div class="real-time-data">
      <div>电压: <span id="voltage">0</span> V</div>
      <div>电流: <span id="current">0</span> A</div>
      <div>功率: <span id="power">0</span> W</div>
      <div>耗电量: <span id="mah">0</span> mAH</div>
    </div>

    <!-- 实时曲线展示 -->
    <div class="chart-container" id="chart-container"></div>

    <!-- 圆形开关按钮 -->
    <label class="switch">
      <input type="checkbox" id="switchButton" onclick="toggleState()">
      <span class="slider"></span>
    </label>
  </div>

  <script>
    // 初始化Highcharts图表
    let voltageData = [];
    let currentData = [];
    let powerData = [];
    let timeData = [];
    let chart;
    function initializeChart() {
      chart = Highcharts.chart('chart-container', {
        chart: {
          type: 'line'
        },
        title: {
          text: '电压、电流、功率实时曲线'
        },
        xAxis: {
          categories: timeData,
          title: {
            text: '时间'
          }
        },
        yAxis: [{
          title: {
            text: '电压 (V)'
          },
          opposite: false
        }, {
          title: {
            text: '电流 (A)'
          },
          opposite: true
        }, {
          title: {
            text: '功率 (W)'
          },
          opposite: false
        }],
        series: [{
          name: '电压 (V)',
          data: voltageData,
          yAxis: 0  // 使用第一个Y轴（电压）
        }, {
          name: '电流 (A)',
          data: currentData,
          yAxis: 1  // 使用第二个Y轴（电流）
        }, {
          name: '功率 (W)',
          data: powerData,
          yAxis: 2  // 使用第三个Y轴（功率）
        }]
      });
    }

    // 获取后端数据并更新UI
    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          const timestamp = new Date().toLocaleTimeString();
          // 更新实时数据显示
          document.getElementById('voltage').innerText = data.voltage;
          document.getElementById('current').innerText = data.current;
          document.getElementById('power').innerText = data.power;
          document.getElementById('mah').innerText = data.mah;

          // 更新曲线数据
          if (timeData.length >= 10) {
            timeData.shift();
            voltageData.shift();
            currentData.shift();
            powerData.shift();
          }
          timeData.push(timestamp);
          voltageData.push(data.voltage);
          currentData.push(data.current);
          powerData.push(data.power);

          chart.xAxis[0].setCategories(timeData);
          chart.series[0].setData(voltageData);
          chart.series[1].setData(currentData);
          chart.series[2].setData(powerData);
        })
        .catch(err => console.error('获取数据失败:', err));
    }

    // 刷新数据
    setInterval(fetchData, 300);

    // 按钮开关状态
    let isOn = true;

    function toggleState() {
      isOn = !isOn;
      const slider = document.querySelector('.slider');
      const button = document.getElementById('switchButton');
      if (isOn) {
        slider.classList.remove('off');
      } else {
        slider.classList.add('off');
      }

      // 发送开关状态到后端
      fetch('/ctrl', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ state: isOn })
      })
        .then(response => response.json())
        .catch(err => console.error('开关状态更新失败:', err));
    }

    // 初始化图表
    initializeChart();
    fetchData(); // 启动第一次数据加载
  </script>
</body>

</html>
)=====";
#endif
