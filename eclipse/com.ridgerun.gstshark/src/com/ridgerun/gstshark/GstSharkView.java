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
import org.swtchart.Chart;

public class GstSharkView extends TmfChartView {


    private static final String SERIES_NAME = "Series";
    private static final String Y_AXIS_TITLE = "Signal";
    private static final String X_AXIS_TITLE = "Time";
    private static final String FIELD = "value"; // The name of the field that we want to display on the Y axis
    private static final String VIEW_ID = "com.ridgerun.gstshark.view";
    private Chart chart;
    private ITmfTrace currentTrace;

    public GstSharkView() {
        super(VIEW_ID);
    }

	@Override
	protected TmfXYChartViewer createChartViewer(Composite arg0) {
		// TODO Auto-generated method stub
		return new GstSharkBaseViewer(arg0,"Scheduling time", "Event time", "time (nanoseconds)");
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