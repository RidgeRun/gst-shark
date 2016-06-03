package com.ridgerun.gstshark;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
//import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.eclipse.tracecompass.tmf.core.event.TmfEvent;
import org.eclipse.tracecompass.tmf.core.request.ITmfEventRequest;
import org.eclipse.tracecompass.tmf.core.request.TmfEventRequest;
import org.eclipse.tracecompass.tmf.core.signal.TmfSignalHandler;
import org.eclipse.tracecompass.tmf.core.signal.TmfTraceSelectedSignal;
import org.eclipse.tracecompass.tmf.core.timestamp.TmfTimeRange;
import org.eclipse.tracecompass.tmf.core.trace.ITmfTrace;
import org.eclipse.tracecompass.tmf.ui.viewers.xycharts.TmfXYChartViewer;
import org.eclipse.tracecompass.tmf.ui.views.TmfChartView;

public class GstSharkCpuUsageView extends TmfChartView {


	private static final String EVENT_NAME = "cpuusage";
	private static final String PLOT_NAME = "CPU Usage";
    private static final String Y_AXIS_TITLE = "CPU load (percentage)";
    private static final String X_AXIS_TITLE = "Event time";
    private static final String VIEW_ID = "com.ridgerun.gstshark.cpuusage";
    private ITmfTrace currentTrace;

    public GstSharkCpuUsageView() {
        super(VIEW_ID);
    }

	@Override
	protected TmfXYChartViewer createChartViewer(Composite arg0) {
		// TODO Auto-generated method stub
		return new GstSharkCpuUsage(arg0,PLOT_NAME, X_AXIS_TITLE, Y_AXIS_TITLE,EVENT_NAME);
	}
	
	@TmfSignalHandler
    public void traceSelected(final TmfTraceSelectedSignal signal) {
        // Don't populate the view again if we're already showing this trace
        if (currentTrace == signal.getTrace()) {
            return;
        }
        currentTrace = signal.getTrace();

        // Create the request to get data from the trace

        TmfEventRequest req = new TmfEventRequest(TmfEvent.class,
                TmfTimeRange.ETERNITY, 0, ITmfEventRequest.ALL_DATA,
                ITmfEventRequest.ExecutionType.BACKGROUND) {

            @Override
            public void handleData(ITmfEvent data) {
                // Called for each event
                super.handleData(data);
            }

            @Override
            public void handleSuccess() {
                // Request successful, not more data available
                super.handleSuccess();
            }

            @Override
            public void handleFailure() {
                // Request failed, not more data available
                super.handleFailure();
            }
        };
        ITmfTrace trace = signal.getTrace();
        trace.sendRequest(req);
    }
}
