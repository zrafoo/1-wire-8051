import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    visible: true
    width: 1920
    height: 1080
    title: "Сергеев Богов Гришаев"

    Canvas {
        id: chartCanvas
        anchors.fill: parent
        anchors.margins: 20

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var data = chartData.data

            if (data.length === 0)
                return

            var maxPoints = 20; // Максимальное количество точек на графике
            var dataToPlot = data.slice(-maxPoints); // Берем последние 20 точек
            var dataLength = dataToPlot.length;

            // Определяем минимальные и максимальные значения для оси Y
            var minY = Math.min.apply(null, dataToPlot);
            var maxY = Math.max.apply(null, dataToPlot);
            var rangeY = maxY - minY || 1; // Чтобы избежать деления на ноль

            // Рисуем оси
            ctx.strokeStyle = "black";
            ctx.lineWidth = 2;

            // Вертикальная ось
            ctx.beginPath();
            ctx.moveTo(40, 0);
            ctx.lineTo(40, height);
            ctx.stroke();

            // Горизонтальная ось
            ctx.beginPath();
            ctx.moveTo(0, height - 40);
            ctx.lineTo(width, height - 40);
            ctx.stroke();

            // Рисуем сетку
            ctx.strokeStyle = "lightgray";
            ctx.lineWidth = 1;
            for (var i = 0; i <= 10; i++) {
                var y = height - 40 - (i * (height - 40) / 10);
                ctx.beginPath();
                ctx.moveTo(40, y);
                ctx.lineTo(width, y);
                ctx.stroke();
            }

            // Рисуем график
            ctx.strokeStyle = "green";
            ctx.lineWidth = 2;
            ctx.beginPath();
            for (var i = 0; i < dataLength; i++) {
                var x = 40 + i * (width - 80) / (maxPoints - 1); // Используем maxPoints для расчета X
                var y = height - 40 - (dataToPlot[i] - minY) * (height - 80) / rangeY;
                if (i === 0)
                    ctx.moveTo(x, y);
                else
                    ctx.lineTo(x, y);
                ctx.fillText(dataToPlot[i].toFixed(2), x, y - 5); // Отображаем значение температуры
            }
            ctx.stroke();

            // Рисуем метки осей
            ctx.fillStyle = "black";
            ctx.font = "bold 14px Arial";
            ctx.fillText("Температура (°C)", width / 2, height - 10);
            ctx.save();
            ctx.rotate(-Math.PI / 2);
            ctx.fillText("Значение", -height / 2 + 20, 20);
            ctx.restore();
        }

        Connections {
            target: chartData
            onDataChanged: {
                chartCanvas.requestPaint();
            }
        }
    }
}
