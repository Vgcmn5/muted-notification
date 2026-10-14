#include "indicator.h"

#include <obs-frontend-api.h>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QCursor>
#include <QScreen>
#include <QTimer>
#include <QTime>
#include <QColor>

class OverlayWidget : public QWidget {
	QTimer *m_timer;
	bool m_want_show = false;
	int m_timeout_ms = 0;
	int m_indicator_size = 0;
	int m_position = 0; // 0=TL, 1=TR, 2=BL, 3=BR, 4=Center
	uint64_t m_start_time = 0;
	QColor m_color = Qt::red;
public:
	OverlayWidget(QWidget *parent)
		: QWidget(parent)
	{
		setWindowFlags(Qt::FramelessWindowHint |
			       Qt::WindowStaysOnTopHint | Qt::Tool);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_ShowWithoutActivating);

		setFixedSize(50, 50);
		setVisible(false);
		m_timer = new QTimer(this);

		connect(m_timer, &QTimer::timeout, this, &OverlayWidget::tick);
		m_timer->start(100);
	}

	void queue_show(int timeout, int size, uint32_t color, int position)
	{
		m_timeout_ms = timeout;
		m_indicator_size = size;
		m_position = position;
		
		int r = color & 0xFF;
		int g = (color >> 8) & 0xFF;
		int b = (color >> 16) & 0xFF;
		m_color = QColor(r, g, b);

		m_want_show = true;
	}

protected:
	void updateIndicatorPosition()
	{
		QPoint cursorPos = QCursor::pos();
		QScreen *screen = QGuiApplication::screenAt(cursorPos);
		if (screen) {
			QRect screenGeometry = screen->geometry();
			int margin = 10;
			int sz = m_indicator_size;

			switch (m_position) {
				case 0: // Top-Left
					move(screenGeometry.topLeft() + QPoint(margin, margin));
					break;
				case 1: // Top-Right
					move(screenGeometry.topRight() + QPoint(-sz - margin, margin));
					break;
				case 2: // Bottom-Left
					move(screenGeometry.bottomLeft() + QPoint(margin, -sz - margin));
					break;
				case 3: // Bottom-Right
					move(screenGeometry.bottomRight() + QPoint(-sz - margin, -sz - margin));
					break;
				case 4: // Center
					move(screenGeometry.center() + QPoint(-sz / 2, -sz / 2));
					break;
				default:
					move(screenGeometry.topLeft() + QPoint(margin, margin));
					break;
			}
		}
	}

	void paintEvent(QPaintEvent *event) override
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(Qt::NoPen); // Removes the tiny black outline
		painter.setBrush(QBrush(m_color)); 
		painter.drawEllipse(0, 0, width(), height());
	}

protected slots:

	void tick()
	{
		if (m_want_show) {
			m_start_time = QDateTime::currentMSecsSinceEpoch();
			m_want_show = false;
			setFixedSize(m_indicator_size, m_indicator_size);
			updateIndicatorPosition();
			setVisible(true);
		}

		if (m_timeout_ms > 0) {
			uint64_t current_time =
				QDateTime::currentMSecsSinceEpoch();
			if (current_time - m_start_time > m_timeout_ms) {
				setVisible(false);
				m_timeout_ms = 0;
			}
		}
	}
};

static OverlayWidget *overlay = nullptr;

void indicator_init()
{
	QWidget* main_window = static_cast<QWidget*>(obs_frontend_get_main_window());
	if (!main_window)
		return;

	overlay = new OverlayWidget(main_window);
}

void indicator_show(int timeout_ms, int indicator_size, uint32_t color, int position)
{
	if (overlay) {
		overlay->queue_show(timeout_ms, indicator_size, color, position);
	}
}