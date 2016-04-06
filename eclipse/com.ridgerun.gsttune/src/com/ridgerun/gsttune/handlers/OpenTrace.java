package com.ridgerun.gsttune.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.WorkbenchException;
import org.eclipse.ui.handlers.HandlerUtil;
import org.eclipse.ui.handlers.IHandlerService;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.IWorkbenchPartSite;

public class OpenTrace extends AbstractHandler {

	public OpenTrace() {
	}

	/**
	 * the command has been executed, so extract extract the needed information
	 * from the application context.
	 */
	public Object execute(ExecutionEvent event) throws ExecutionException {

        try 
		{
		   PlatformUI.getWorkbench().showPerspective("org.eclipse.linuxtools.lttng2.kernel.ui.perspective",
		         PlatformUI.getWorkbench().getActiveWorkbenchWindow());
		} 
		catch (WorkbenchException e) 
		{
		   e.printStackTrace();
		}

        IWorkbenchPart apart = HandlerUtil.getActivePart(event);
        IWorkbenchPartSite site = apart.getSite();
        IHandlerService handlerService = (IHandlerService) site.getService(IHandlerService.class);
        try {
          handlerService.executeCommand("org.eclipse.linuxtools.tmf.ui.openFile", null);
          } catch (Exception ex) {
            throw new RuntimeException("org.eclipse.linuxtools.tmf.ui.openFile not found");
            // Give message
            }


        return null;
	}
}